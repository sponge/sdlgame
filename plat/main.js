/// <reference path="./decs.d.ts" />
import * as Draw from 'draw';
import * as SLT from 'slate2d';
import * as Assets from 'assets';
import Camera from './js/camera.js';
import LDTK from './js/ldtk.js';

class Entity {
  type = 'default';
  x = 0;
  y = 0;
  sprite = 0;

  update(_dt) {}
  draw() {}
}

class Player extends Entity {
  t = 0;
  constructor() {
    super()
    this.type = 'player';
    this.sprite = Assets.find('dogspr');
  }

  update(dt) {
    this.t += dt;
    if (SLT.buttonPressed(0)) this.y -= 1;
    if (SLT.buttonPressed(1)) this.y += 1;
    if (SLT.buttonPressed(2)) this.x -= 1;
    if (SLT.buttonPressed(3)) this.x += 1;
  }

  draw() {
    Draw.setColor(255, 255, 255, 255);
    Draw.sprite(this.sprite, Math.floor(this.t * 12) % 6, this.x, this.y, 1, 0, 1, 1);
  }
}

class Main {
  res = {w: 384, h: 216}
  canvas = undefined;
  dog = undefined;
  dogSpr = undefined;
  state = {
    t: 0,
    entities: [],
    mapName: '',
  };
  map = undefined;
  backgrounds = [];
  clouds = [];
  camera = new Camera(this.res.w, this.res.h);
  entMap = {
    'player': Player
  };

  save() {
    return JSON.stringify(this.state);
  }

  start(initialState) {
    this.canvas = Assets.load({
      name: 'canvas',
      type: 'canvas',
      width: this.res.w,
      height: this.res.h
    });

    SLT.registerButtons(['up', 'down', 'left', 'right']);
  
    this.dog = Assets.load({
      name: 'dog',
      type: 'image',
      path: 'gfx/dog.png'
    });
  
    this.dogSpr = Assets.load({
      name: 'dogspr',
      type: 'sprite',
      path: 'gfx/dog.png',
      spriteWidth: 22,
      spriteHeight: 16,
      marginX: 0,
      marginY: 0,
    });

    this.backgrounds = [...Array(3).keys()].map(i => {
      const name = `gfx/grassland_bg${i}.png`;
      const id = Assets.load({type: 'image', name, path: name});
      const {w, h} = Assets.imageSize(id);
      return { id, w, h };
    });

    const cloudX = [50, 100, 150];
    const cloudY = [15, 45, 30];
    this.clouds = [...Array(3).keys()].map(i => {
      const name = `gfx/grassland_cloud${i}.png`;
      const id = Assets.load({type: 'image', name, path: name});
      const {w, h} = Assets.imageSize(id);
      return { id, w, h, x: cloudX[i], y: cloudY[i] };
    });

    if (initialState) {
      this.state = JSON.parse(initialState);
      this.state.entities = this.state.entities.map(ent => Object.assign(new this.entMap[ent.type], ent));
    } else {
      const player = new Player();
      player.x = 100;
      player.y = 100;
      this.state.entities.push(player);
      this.state.mapName = 'maps/0000-Level_0.ldtkl';
    }

    const src = JSON.parse(SLT.readFile(this.state.mapName));
    this.map = new LDTK(src);

    this.camera.constrain(0, 0, this.map.widthPx, this.map.heightPx);
  };

  update(dt) {
    this.state.t += dt;
    SLT.showObj('main class', this);

    this.state.entities.forEach(ent => ent.update(dt));
    const player = this.state.entities[0];
    this.camera.window(player.x, player.y, 20, 20);
  };

  draw() {
    Draw.clear(0, 0, 0, 255);

    Draw.useCanvas(this.canvas);
    Draw.clear(41, 173, 255, 255);
  
    const { res } = this;
    const t = this.state.t;
  
    // parallax bgs
    this.backgrounds.forEach((bg, i) => {
      const speed = (i+1) * 0.25;
      const x = Math.floor(((0 - this.camera.x) * speed) % bg.w);
      const camY = 1 - (this.camera.y / (this.camera.con.h - res.h));
      const y = Math.floor(res.h - bg.h + (camY * 20));
      Draw.image(bg.id, x, y, 0, 0, 1, 0, 0, 0);
      Draw.image(bg.id, x + bg.w, y, 0, 0, 1, 0, 0, 0);
    });

    // clouds which scroll, no parallax
    this.clouds.forEach((bg, i) => {
      const speed = (i+1) * 6;
      const x = res.w + (bg.x - t * speed) % (res.w + bg.w);
      Draw.image(bg.id, x, bg.y, 0, 0, 1, 0, 0, 0);
    });
  
    // running dog
    const x = Math.floor((t * 50) % (res.w + 22) - 22);
    const y = Math.floor(Math.sin(x / 50) * 5 + 167);
    Draw.setColor(255, 255, 255, 255);
    Draw.sprite(this.dogSpr, Math.floor(t * 12) % 6, x, y, 1, 0, 1, 1);
    Draw.setColor(255, 255, 255, 128);
  
    this.camera.drawStart();
    //Draw.tilemap(this.tiles.tilesetHandle, 0, 0, this.tiles.width, this.tiles.height, this.tiles.data);

    // random triangle and lines
    Draw.tri(10, 20, 400, 400, 400, 100, true);
    Draw.setColor(128, 0, 0, 255);
    Draw.line(0, 0, res.w, res.h);
    Draw.line(0, 0, 200, 200);

    // tilemap and entities
    Draw.setColor(255, 255, 255, 255);
    this.map.draw('BGDecoration');
    this.state.entities.forEach(ent => ent.draw());
    this.map.draw('BGTiles');
    this.map.draw('Collision');

    this.camera.drawEnd();

    // draw the canvas into the center of the window
    const screen = SLT.resolution();
    const scale = Math.floor(screen.h / res.h);
    Draw.resetCanvas();
    Draw.setColor(255, 255, 255, 255);
    Draw.image(this.canvas, (screen.w - (res.w * scale)) / 2, (screen.h - (res.h * scale)) / 2, res.w, res.h, scale, 0, 0, 0);

    Draw.submit();
  };
}

export default Main;