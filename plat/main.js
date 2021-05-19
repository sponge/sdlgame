import { testmodule, testmodule2, testmodule3, testmodule4 } from "./test/testmodule.js";
import * as Draw from "draw";
import * as SLT from "slate2d";
import * as Assets from "assets";

class Main {
  constructor() {
    this.dog = undefined;
    this.dogSpr = undefined;
    this.state = {t: 0};

    console.log("constructed!");
  }

  save() {
    return JSON.stringify(this.state);
  }

  start(initialState) {
    console.log("start");
    console.log("platform is " + SLT.platform);
    SLT.registerButtons(['up', 'down', 'left', 'right']);
  
    if (initialState) {
      this.state = JSON.parse(initialState);
    }
  
    this.dog = Assets.load({
      name: 'dog',
      type: 'image',
      path: 'gfx/dog.png'
    });
  
    this.dogSpr = Assets.load({
      name: 'dogspr',
      type: 'sprite',
      path: 'gfx/dog.png',
      spriteWidth: 16,
      spriteHeight: 16,
      marginX: 0,
      marginY: 0,
    })

  };

  draw() {
    Draw.clear(0, 0, 0, 255);
  
    const mouse = SLT.mouse();
    const res = SLT.resolution();
  
    let t = this.state.t;
  
    const x = Math.floor((t * 100) % res.w);
    const y = Math.floor(Math.sin(x / 50) * 100 + 200);
    const sz = Math.cos(t) * 32 + 32
    Draw.rect(x, y, sz, sz, true);
  
    Draw.image(this.dog, 0, 0, 32, 32, 1, 0, 0, 0);
    Draw.tri(10, 20, 400, 400, 400, 100, true);
    Draw.setColor(128, 0, 0, 255);
    Draw.line(0, 0, res.w, res.h);
    Draw.setColor(255, 255, 255, 128);
    Draw.rect(mouse.x, mouse.y, 16, 16);
    Draw.submit();
    SLT.printWin("test", "key", "val");
    SLT.printWin("test", "x", x);
    SLT.printWin("test", "y", y);
  };
  
  update(dt) {
    this.state.t += dt;
    testmodule(1);
    testmodule4(4);
  
    // if (t > 3) {
    //   throw new Error("test exception");
    // }
  };
}

export default Main;

/*
let dog, dogSpr;
let state = {t: 0};

function save() {
  return JSON.stringify(state);
}

function start(initialState) {
  console.log("start");
  console.log("platform is " + SLT.platform);
  SLT.registerButtons(['up', 'down', 'left', 'right']);

  if (initialState) {
    state = JSON.parse(initialState);
  }

  dog = Assets.load({
    name: 'dog',
    type: 'image',
    path: 'gfx/dog.png'
  });

  dogSpr = Assets.load({
    name: 'dogspr',
    type: 'sprite',
    path: 'gfx/dog.png',
    spriteWidth: 16,
    spriteHeight: 16,
    marginX: 0,
    marginY: 0,
  })
};

function draw() {
  Draw.clear(0, 0, 0, 255);

  const mouse = SLT.mouse();
  const res = SLT.resolution();

  let t = state.t;

  const x = Math.floor((t * 100) % res.w);
  const y = Math.floor(Math.sin(x / 50) * 100 + 200);
  const sz = Math.cos(t) * 32 + 32
  Draw.rect(x, y, sz, sz, true);

  Draw.image(dog, 0, 0, 32, 32, 1, 0, 0, 0);
  Draw.tri(10, 20, 400, 400, 400, 100, true);
  Draw.setColor(128, 0, 0, 255);
  Draw.line(0, 0, res.w, res.h);
  Draw.setColor(255, 255, 255, 128);
  Draw.rect(mouse.x, mouse.y, 16, 16);
  Draw.submit();
  SLT.printWin("test", "key", "val");
  SLT.printWin("test", "x", x);
  SLT.printWin("test", "y", y);
};

function update(dt) {
  state.t += dt;
  testmodule(1);
  testmodule4(4);

  // if (t > 3) {
  //   throw new Error("test exception");
  // }
};

export default { draw, update, start, save };
*/