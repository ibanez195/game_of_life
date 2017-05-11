# Game of Life

Conway's game of life implemented in ncurses

## Arguments
- -c COLOR

  Make cells use specified color
- -s SPEED

  Starting speed. Integer from 0-10
- -f FILE

  Text file should be formatted as a rectangle of 1s and 0s with 1s representing a live cell e.g.
```
  1111111111
  1000000001
  1011111101
  1010000101
  1010110101
  1010110101
  1010000101
  1011111101
  1000000001
  1111111111
```

## Controls
- Pause/Unpause - "return"
- Slow Simulation Speed - "-"
- Increase Simulation Speed - "="

### While paused
- Toggle cell - "space or click"
- Move Cursor - "h,j,k,l"
- Step one generation - "s"
- Export current state to file - "e"

### Screenshot
![game_of_life](https://cloud.githubusercontent.com/assets/6550505/25973235/324181ac-3671-11e7-8aee-b9391c6cc9df.png))]
