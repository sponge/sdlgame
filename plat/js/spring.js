import * as Assets from 'assets';
import Entity from './entity.js';
import Dir from './dir.js';
import CollisionType from './collisiontype.js';
class Spring extends Entity {
    sprite = Assets.find('spring');
    collidable = CollisionType.Platform;
    constructor(args) {
        super(args);
    }
    update(dt) {
    }
    draw() {
        //Draw.setColor(255, 255, 0, 255);
        //Draw.rect(this.pos[0], this.pos[1], this.size[0], this.size[1], false);
        super.draw();
    }
    collide(other, dir) {
        if (dir != Dir.Up)
            return;
        other.vel[1] = -9;
    }
}
export default Spring;
