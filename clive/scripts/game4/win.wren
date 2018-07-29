import "engine" for Draw, Trap, Asset, Color, Button, Align
import "britnames" for BritNames
import "debug" for Debug
import "soundcontroller" for SoundController

class Game4Win {
   nextScene { _nextScene }

   construct new(params) {
      Asset.clearAll()

      _nextScene = null
      _time = 0

      _music = Asset.create(Asset.Sound, "game4_title", "sound/game4_title.ogg")
      _font = Asset.create(Asset.Font, "body", "fonts/Roboto-Regular.ttf")

      _britNames = BritNames.new()
      for (i in 1...300) {
         Debug.printLn(_britNames.next())
      }

      _lines = [
         [0, 720, 1280, "Great job, soldier! Those goats won't be a problem no more!"],
         [0, 760, 1280, "Thanks to all of our backers for making this game possible:"],
         [0, 800, 1280, ""],
      ]

      Asset.loadAll()

      SoundController.playMusic(_music)
   }

   shutdown() {
      SoundController.stopMusic()
   }

   update(dt) {
      _time = _time + dt

      if (Trap.keyPressed(Button.Start, 0, -1)) {
         _nextScene = "gameselect"
      }

      for (i in _lines.count-1..0) {
         var line = _lines[i]
         line[1] = line[1] - 1

         if (line[1] < -32) {
            _lines.removeAt(i)
         }
      }

      if (_lines[-1][1] < 720 - 32) {
         _lines.add([0, 720, 640, _britNames.next()])
         _lines.add([640, 720, 640, _britNames.next()])

      }
   }

   draw(w, h) {
      Draw.clear()
      Draw.transform(h/720, 0, 0, h/720, 0, 0)

      Draw.setTextStyle(_font, 36, 1.0, Align.Center+Align.Top)

      for (line in _lines) {
         Draw.text(line[0], line[1], line[2], line[3])
      }

      Draw.submit()
   }
}