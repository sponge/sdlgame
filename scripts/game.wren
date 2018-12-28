import "random" for Random

import "engine" for Draw, Asset, TileMap, Trap, Button
import "math" for Math
import "camera" for Camera

import "player" for Player
import "mine" for Mine
import "minetext" for MineText
import "meter" for Meter

class Game {
   nextScene { _nextScene }
   nextScene=(params) { _nextScene = params }

   entities { _entities }
   cam { _cam }
   rnd { _rnd }
   meter { _meter }

   construct new(params) {
      _icons = Asset.create(Asset.Sprite, "icons", "gfx/icons.png")
      Asset.spriteSet(_icons, 16, 16, 0, 0)

      _meter = Meter.new()
      _cam = Camera.new(16, 16, 320, 180)
      _rnd = Random.new()
      _player = Player.new(this, {}, 220, 50)

      _t = 0
      _entities = [_player]
      _generatedX = 0 // how far in the world we've generated level parts

      _modes = ["rain", "show", "minefield", "ashes"]

      _generateMode = "minefield"

      // rain generator
      _nextRainTick = 0

      _uiEntities = []

      Asset.loadAll()
   }

   generateRain(subtype) {
      var cx = _cam.toWorld(0,0)[0]

      if (cx <= _generatedX) {
         _generatedX = cx - _cam.w * 1.25
      }

      if (_t < _nextRainTick) {
         return
      }

      _nextRainTick = _t + 20
      
      var d = [0, 0]
      var y = 0

      if (subtype == "rain") {
         d = [_rnd.float(0.05, 0.25), _rnd.float(0.4, 0.5)]
         y = -16
      } else if (subtype == "snow") {
         d = [_rnd.float(-0.25, 0.25), _rnd.float(0.2, 0.4)]
         y = -16
      } else if (subtype == "ashes") {
         d = [_rnd.float(-0.25, 0.25), _rnd.float(-0.2, -0.4)]
         y = _cam.h
      }

      var mine = Mine.new(this, {"sprite": _icons}, _rnd.int(cx - 64, cx+_cam.w), y, d[0], d[1])
      _entities.add(mine)
   }

   generateMinefield() {
      // only run once
      var cx = _cam.toWorld(0,0)[0]
      if (cx > _generatedX) {
         return
      }

      // the new chunk will be a bit longer than the camera view so we don't pop in
      _generatedX = cx - _cam.w * 1.25

      var y = 0
      var x = cx
      // generate a random chance of an obstacle in every 24px grid
      while (y < _cam.h) {
         while (x > _generatedX) {
            if (_rnd.int(5) == 0) {
               var mine = Mine.new(this, {"sprite": _icons}, x + _rnd.int(8), y + _rnd.int(8), 0, 0)
               _entities.add(mine)
            }
            x = x - 24
         }
         y = y + 24
         x = cx
      }
   }

   update(dt) {
      _t = _t + dt

      // if we've past the point where we need to switch, pick a new random section
      var cx = _cam.toWorld(0,0)[0]
      if (cx <= _generatedX) {
         _generateMode = _rnd.sample(_modes)
      }

      // run the level generator tick
      if (_generateMode == "minefield") {
         generateMinefield()
      } else if (_generateMode == "rain" || _generateMode == "snow" || _generateMode == "ashes") {
         generateRain(_generateMode)
      }

      // if the player isn't on the starting platform, autoscroll the camera
      if (_player.launched) {
         _cam.move(_cam.x - 0.25, 0)
      }

      // update each entity and kill them if they're off camera
      for (ent in _entities) {
         ent.think(dt)
         if (ent.x > _cam.x + _cam.w) {
            ent.die(false)
         }
      }

      // trim out dead entities from the list
      _entities = _entities.where {|c| !c.dead }.toList

      // update ui entities
      for (ent in _uiEntities) {
         ent.think(dt)
      }
      _uiEntities = _uiEntities.where {|c| !c.dead }.toList
      _meter.think(dt)
   }

   onMineHit(mine) {
      _uiEntities.add(MineText.new(mine.spr, 140, 160))
   }

   draw(w, h) {
      Draw.clear()
      Draw.resetTransform()
      Draw.scale(h / _cam.h)

      Draw.translate(-_cam.x, -_cam.y)

      for (ent in _entities) {
         ent.draw()
      }

      // draw the ui elements on top of everything without the camera translation
      Draw.translate(_cam.x, _cam.y)
      for (ent in _uiEntities) {
         ent.draw()
      }
      _meter.draw()
   }

   shutdown() {
      Asset.clearAll()
   }  
}