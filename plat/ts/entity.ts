import * as SLT from 'slate2d';
import * as Draw from 'draw';
import { clamp, rectIntersect } from './util.js';
import Dir from './dir.js';
import Main from './main.js';
import Tiles from './tiles.js';
import CollisionType from './collisiontype.js';

class Entity {
  type = 'default';
  pos = [0, 0];
  size = [0, 0];
  vel = [0, 0];
  drawOfs = [0, 0];

  remainder = [0, 0];
  sprite = 0;
  frame = 0;
  collidable = CollisionType.Enabled;

  update(_dt: number): void { }
  draw() {
    Draw.setColor(255, 255, 255, 255);
    Draw.sprite(this.sprite, this.frame, this.pos[0] + this.drawOfs[0], this.pos[1] + this.drawOfs[1], 1, 0, 1, 1);
  }

  collideAt(x: number, y: number, dir: Dir) {
    // FIXME: GC?
    const corners = [
      [x, y],
      [x + this.size[0] - 1, y],
      [x, y + this.size[1] - 1],
      [x + this.size[0] - 1, y + this.size[1] - 1]
    ];

    const bottomMiddle = [x + this.size[0] / 2, corners[2][1]];

    // FIXME: need a reference to the world, but don't want to pass it in then state will have a cyclic reference
    const main = ((globalThis as any).main as Main);
    const layer = main.map.layersByName.Collision;
    const entities = main.state.entities;

    // iterate through all entities looking for a collision
    for (let other of entities) {
      if (other == this) continue;
      if (other.collidable == CollisionType.Disabled) continue;

      const intersects = rectIntersect(x, y, this.size[0], this.size[1], other.pos[0], other.pos[1], other.size[0], other.size[1]);
      if (other.collidable == CollisionType.Enabled && intersects) {
        return true;
      }
      else if (other.collidable == CollisionType.Platform && dir == Dir.Down && intersects && corners[2][1] == other.pos[1]) {
        return true;
      }
    }

    // check bottom middle point if its in a slope
    const tx = Math.floor(bottomMiddle[0] / layer.tileSize);
    const ty = clamp(Math.floor(bottomMiddle[1] / layer.tileSize), 0, layer.height);
    const tid = layer.tiles[ty * layer.width + tx];
    if (tid == Tiles.SlopeL || tid == Tiles.SlopeR) {
      const localX = bottomMiddle[0] % layer.tileSize;
      const localY = bottomMiddle[1] % layer.tileSize;
      const minY = tid == Tiles.SlopeR ? localX : layer.tileSize - localX;

      return localY >= minY;
    }

    // check against tilemap
    // iterate through corners. note this will currently break if entities are > tileSize
    for (let corner of corners) {
      const tx = Math.floor(corner[0] / layer.tileSize);
      const ty = clamp(Math.floor(corner[1] / layer.tileSize), 0, layer.height);
      const tid = layer.tiles[ty * layer.width + tx];
      //if there's a tile in the intgrid...
      if (tx < 0 || tx >= layer.width || tid !== Tiles.Empty) {
        if (tid == Tiles.Dirtback) {
          continue;
        }

        // if it's a ground sloped tile, only bottom middle pixel should collide with it
        if (tid == Tiles.SlopeL || tid == Tiles.SlopeR) {
          continue;
        }

        // if it's a platform, check if dir is down, and only block if bottom of entity
        // intersects with the first pixel of the platform block
        if (tid == Tiles.Platform) {
          if (dir == Dir.Down && corner[1] == corners[2][1] && corner[1] % layer.tileSize == 0) {
            return true;
          }
          continue;
        }

        // TODO: set some sort of collision response object on the entity to prevent alloc new objs?
        return true;
      }
    }

    return false;
  }

  __move(dim: number, amt: number) {
    this.remainder[dim] += amt;
    let move = Math.floor(this.remainder[dim]);

    if (move == 0) {
      return true;
    }

    this.remainder[dim] -= move;
    const sign = Math.sign(move);

    while (move != 0) {
      const check = this.pos[dim] + sign;
      const dir = dim == 0 ? (sign > 0 ? Dir.Right : Dir.Left) : (sign > 0 ? Dir.Down : Dir.Up);
      const collision = dim == 0 ? this.collideAt(check, this.pos[1], dir) : this.collideAt(this.pos[0], check, dir);
      if (!collision) {
        this.pos[dim] += sign;
        move -= sign;
      } else {
        // step up 1 pixel to check for slope
        if (dim == 0) {
          if (!this.collideAt(check, this.pos[1] - 1, Dir.Up)) {
            this.pos[0] += sign;
            this.pos[1] -= 1;
            move -= sign;
            continue;
          }
        }
        return false;
      }
    }

    return true;
  }

  moveX(amt: number) {
    return this.__move(0, amt);
  }

  moveY(amt: number) {
    return this.__move(1, amt);
  }
}

export default Entity;