import * as Draw from 'draw';
import * as SLT from 'slate2d';
import * as Assets from 'assets';
import Buttons from './buttons.js';
import Entity from './entity.js';
import { clamp } from './util.js';
import Dir from './dir.js';
import Tiles from './tiles.js';
import Phys from './phys.js';
const slopes = [Tiles.SlopeL, Tiles.SlopeR];
class Player extends Entity {
    // entity definition
    type = 'Player';
    sprite = Assets.find('dogspr');
    drawOfs = [-5, -2];
    spawnPos;
    // entity state
    disableControls = false;
    disableMovement = false;
    pMeter = 0;
    fallingFrames = 0;
    jumpHeld = false;
    jumpHeldFrames = 0;
    facing = 1;
    constructor(args) {
        super(args);
        this.spawnPos = [...this.pos];
    }
    die() {
        this.pos = [...this.spawnPos];
    }
    getJumpHeight(speed) {
        speed = Math.abs(speed);
        for (const [checkSpeed, height] of Phys.jumpHeights) {
            if (Math.abs(speed) >= checkSpeed) {
                return height;
            }
        }
        return Phys.jumpHeights[0][1];
    }
    update(ticks, dt) {
        this.frame = ticks / 8 % 6;
        const dir = this.disableControls ? 0 : SLT.buttonPressed(Buttons.Left) ? -1 : SLT.buttonPressed(Buttons.Right) ? 1 : 0;
        const jumpPress = this.disableControls ? false : SLT.buttonPressed(Buttons.Jump);
        const shootPress = this.disableControls ? false : SLT.buttonPressed(Buttons.Shoot);
        const slidePress = this.disableControls ? false : SLT.buttonPressed(Buttons.Down);
        let grounded = this.vel[1] >= 0 && this.collideAt(this.pos[0], this.pos[1] + 1, Dir.Down);
        // checking again because if we're standing on something it should trigger
        if (grounded && this.collideEnt) {
            this.collideEnt.collide(this, Dir.Up);
            grounded = this.vel[1] >= 0 && this.collideAt(this.pos[0], this.pos[1] + 1, Dir.Down);
        }
        // if we're still on the ground, blank out the decimal
        if (grounded) {
            this.remainder[1] = 0;
        }
        // set direction for bullets and sprite drawing
        this.facing = dir != 0 ? dir : this.facing;
        // track frames since leaving platform for late jump presses
        this.fallingFrames = grounded ? 0 : this.fallingFrames + 1;
        // let players jump a few frames early but don't let them hold the button down
        this.jumpHeldFrames = jumpPress ? this.jumpHeldFrames + 1 : 0;
        if (!jumpPress && this.jumpHeld) {
            this.jumpHeld = false;
        }
        // apply gravity if not on the ground. different gravity values depending on holding jump
        this.vel[1] = grounded ? 0 : this.vel[1] + (this.jumpHeld ? Phys.heldGravity : Phys.gravity);
        // slide on ground if holding down
        // y vel handled by slope snapping near end of move
        if (!this.jumpHeld && slidePress && slopes.includes(this.collideTile)) {
            const slideDir = this.collideTile == Tiles.SlopeL ? -1 : 1;
            this.vel[0] += slideDir * Phys.slideAccel;
            this.remainder = [0, 0];
        }
        else if (slidePress || (dir == 0 && this.vel[0] != 0 && grounded)) {
            // if not pushing anything, slow down if on the ground
            this.vel[0] += Phys.friction * -Math.sign(this.vel[0]);
        }
        else if (dir != 0) {
            // if holding a direction, figure out how fast we should try and go
            const speed = Math.sign(dir * this.vel[0]) == -1 ? Phys.skidAccel : Phys.accel;
            this.vel[0] += speed * dir;
        }
        // if jump is held, and player has let go of it since last jump
        if (jumpPress && !this.jumpHeld) {
            // allow the jump if:
            // - they're on the ground, and haven't been holding for too long
            // - they're not on the ground, but have recently been on the ground
            if ((grounded && this.jumpHeldFrames < Phys.earlyJumpFrames) || (!grounded && this.fallingFrames < Phys.lateJumpFrames)) {
                const height = this.getJumpHeight(this.vel[0]);
                this.vel[1] = -height;
                this.jumpHeld = true;
                grounded = false;
                // this.jumpHnd = SLT.sndPlay(this.jumpSound); // TODO: audio
            }
        }
        // increment the p-meter if you're on the ground and going fast enough
        if (Math.abs(this.vel[0]) >= Phys.runSpeed && grounded) {
            this.pMeter += 2;
        }
        else if (grounded || this.pMeter != Phys.pMeterCapacity) {
            // tick down the p-meter, but don't if you're at 100% and midair
            this.pMeter -= 1;
        }
        this.pMeter = clamp(this.pMeter, 0, Phys.pMeterCapacity);
        // hard cap speed values
        if (this.pMeter == Phys.pMeterCapacity) {
            this.vel[0] = clamp(this.vel[0], -Phys.maxSpeed, Phys.maxSpeed);
        }
        else {
            this.vel[0] = clamp(this.vel[0], -Phys.runSpeed, Phys.runSpeed);
        }
        this.vel[1] = Math.min(this.vel[1], Phys.terminalVelocity);
        // move x first, then move y
        if (!this.disableMovement) {
            if (!this.moveX(this.vel[0])) {
                this.vel[0] = this.remainder[0] = 0;
            }
            // snap to ground if in a slope
            this.collideAt(this.pos[0], this.pos[1] + 1, Dir.Down);
            const velY = grounded && this.anyInSlope ? 10 : this.vel[1];
            // this.moveY may alter our velocity, so double check vel before zeroing it out
            if (!this.moveY(velY) && Math.sign(velY) == Math.sign(this.vel[1])) {
                this.vel[1] = this.remainder[1] = 0;
            }
        }
    }
    draw() {
        Draw.setColor(255, 255, 255, 255);
        // Draw.rect(this.pos[0], this.pos[1], this.size[0], this.size[1], false);
        Draw.sprite(this.sprite, this.frame, this.pos[0] + this.drawOfs[0], this.pos[1] + this.drawOfs[1], 1, this.facing > 0 ? 0 : 1, 1, 1);
    }
}
export default Player;
