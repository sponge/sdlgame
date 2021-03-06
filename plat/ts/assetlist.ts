import * as Assets from 'assets';

const AssetList: Asset[] = [
  {
    name: 'dogspr',
    type: 'sprite',
    path: 'gfx/dog.png',
    spriteWidth: 22,
    spriteHeight: 16,
    marginX: 0,
    marginY: 0,
  },
  {
    name: 'pmeter',
    type: 'sprite',
    path: 'gfx/pmeter.png',
    spriteWidth: 10,
    spriteHeight: 14,
    marginX: 0,
    marginY: 0,
  },
  {
    type: 'sprite',
    name: 'coin',
    path: 'gfx/coin.png',
    marginX: 0,
    marginY: 0,
    spriteWidth: 14,
    spriteHeight: 14,
  },
  {
    name: 'blueFont',
    type: 'bitmapfont',
    path: 'gfx/panicbomber_blue.png',
    glyphs: ' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~',
    glyphWidth: 8,
    charSpacing: 0,
    spaceWidth: 8,
    lineHeight: 8,
  },
  {
    type: 'sprite',
    name: 'spring',
    path: 'gfx/spring.png',
    marginX: 0,
    marginY: 0,
    spriteWidth: 16,
    spriteHeight: 16,
  },
  {
    type: 'sprite',
    name: 'bird',
    path: 'gfx/bird.png',
    marginX: 0,
    marginY: 0,
    spriteWidth: 16,
    spriteHeight: 16,
  },
  {
    type: 'sprite',
    name: 'slime',
    path: 'gfx/slime.png',
    marginX: 0,
    marginY: 0,
    spriteWidth: 16,
    spriteHeight: 16,
  },
  {
    type: 'sprite',
    name: 'bub',
    path: 'gfx/bub.png',
    marginX: 0,
    marginY: 0,
    spriteWidth: 16,
    spriteHeight: 16,
  },
  {
    type: 'sprite',
    name: 'ghost',
    path: 'gfx/ghost.png',
    marginX: 0,
    marginY: 0,
    spriteWidth: 18,
    spriteHeight: 16,
  },
  {
    type: 'sprite',
    name: 'puff',
    path: 'gfx/puff.png',
    marginX: 0,
    marginY: 0,
    spriteWidth: 16,
    spriteHeight: 16,
  },
  {
    type: 'sprite',
    name: 'hermit',
    path: 'gfx/hermit.png',
    marginX: 0,
    marginY: 0,
    spriteWidth: 19,
    spriteHeight: 13,
  },
];

function loadAllAssets() {
  for (const asset of AssetList) {
    Assets.load(asset);
  }
}

export default loadAllAssets;