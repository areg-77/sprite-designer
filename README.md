# Sprite Designer

A tool for designing sprites mostly for games but using only the **Terminal**.


## Features

- Drawing the Sprite (see in [Controls](#controls))
- Animating the sprite / Previewing the Animation (see in [Animations](#animations))
- Adding Sound effects to the Keyframes (see in [Sound effects](#sound-effects))
- Defining parts of the sprite with Collisions or Triggers(with channels) (see in [Configuration Mode](#configuration-mode))

------

### Examples

![city animation](https://github.com/user-attachments/assets/819cb2c7-351c-4019-bb2c-d09717df0679)
![smiley](https://github.com/user-attachments/assets/e2ee391b-ec52-4c90-a1e3-eaba3f8aeaca)
![waterfall](https://github.com/user-attachments/assets/6f973871-1c43-4ee6-8a24-0e4288cf3843)

------

### Controls

### To navigate in the **Menu** use `arrow keys` and `Enter`/`Esc`

### This app uses `*.spr` for **sprite files**

| Key                             | Tab      | Description                                                                                                       |
| :------------------------------ | :------- | :---------------------------------------------------------------------------------------------------------------- |
| **`F5`**                        |          | Refresh the viewport                                                                                              |
| **`LMB`**                       |          | Draw                                                                                                              |
| **`Ctrl`** + **`LMB`**          |          | Erase                                                                                                             |
| **`Tab`**                       |          | Switch between the **Tabs**: `symbol` and `color`                                                                 |
| **`Q`/`E`**                     | `symbol` | Increase/Decrease the symbol's **ASCII Code**                                                                     |
| **`LCtrl`** + **`Q`/`E`**       | `symbol` | **Fast** Increase/Decrease                                                                                        |
| **`Q`/`E`**                     | `color`  | Increase/Decrease the symbol's **HEX Color**                                                                      |
| **`Q`/`E` + `1`/`2`/`3`**       | `color`  | Increase/Decrease the symbol's **RGB Color** *(1 for **red**, 2 for **blue**, 3 for **green**)*                   |
| **`1` + `2` + `3`**             | `color`  | Random Color                                                                                                      |
| **`F6`**                        | `color`  | Paste the color of the **HEX Color** copied to your `clipboard`                                                   |
| **`LCtrl`** + any color control | `color`  | Apply the same Function to the **background** color of the symbol                                                 |
| **`RCtrl`** + any color control | `color`  | **Fast** Functions                                                                                                |
| **`X`**                         | `color`  | Switch the **foreground** and **background** colors                                                               |
| **`C`**                         |          | Copy the sprite from the current frame                                                                            |
| **`V`**                         |          | Paste the **copied** sprite *(you can move it around with the `mouse` or `arrow keys`)*                           |
| **`Q` + `E`**                   |          | Reset the `symbol code`/`color` to the **default**                                                                |
| **`Alt`** (Hold)                | `color`  | Show the **HEX Color** of the hovered symbol                                                                      |
| **`Alt` + `MMB`**               |          | Move the sprite                                                                                                   |
| **`MMB`**                       | `symbol` | Copy the `symbol code` of the hovered symbol                                                                      |
| **`MMB`**                       | `color`  | Copy the `color` of the hovered symbol                                                                            |
| **`LCtrl` + `MMB`**             | `color`  | Copy the `background color` of the hovered symbol                                                                 |
| **`RCtrl` + `Enter`**           |          | Fill the whole frame with the **foreground color**                                                                |
| **`RCtrl` + `Enter`**           |          | Erase the whole frame                                                                                             |
| **`Alt` + `F8`/`F9`**           |          | Increase/Decrease the **brightness** of the current frame *(clicking on the percentage will resterize the frame)* |
| **`Ctrl` + `Alt` + `F8`/`F9`**  |          | **Small** steps Increase/Decrease the **brightness**                                                              |
| **`F8` + `F9`**                 |          | Reset the **brightness** to default value                                                                         |
| **`*` or `/`**                  |          | Increase/Decrease the **screen's width**                                                                          |
| **`LCtrl` + `*` or `/`**        |          | Increase/Decrease the **screen's height**                                                                         |
| **`RCtrl` + `*` or `/`**        |          | **Fast** Increase/Decrease the **screen size**                                                                    |
| **`+` or `-`**                  |          | Increase/Decrease the characters **font sizeY** *(vertical)*                                                      |
| **`Ctrl` + `+`/`-`**            |          | Increase/Decrease the characters **font sizeX** *(horizontal)*                                                    |
| **`Ctrl` + `Z`**                |          | Undo the **changes**                                                                                              |
| **`M`**                         |          | Mirror the **sprite** *(horizontally)*                                                                            |
| **`Ctrl` + `M`**                |          | Mirror the **sprite** *(vertically)*                                                                              |
| **`Esc`**                       |          | Escape to the **menu** for `saving`, `importing`, `exiting`                                                       |

### If you click on the `HEX Color` it will be shown in `RGB mode`

------

### Configuration Mode

| Key                   | Description                                                    |
| :-------------------- | :------------------------------------------------------------- |
| **`F10`**             | Switch to **Configuration Mode**                               |
| **`Q`/`E`**           | Switch between: `C: (collider)`, `T: (trigger)`, `U: (update)` |
| **`Tab`**             | To set the channel for a `trigger`                             |
| **`RCtrl` + `Enter`** | Fill the whole sprite with `C`, `T` or `U`                     |

#### Explanation:
- **Collider**: areas that **collide** with each other
- **Trigger**: area that when an element enters it a certain function gets called that you can observe by the channel of that **trigger (1-255)**
- **Update**: areas that are marked as **"alive"** so that on the next frame they get redrawn

### NOTE: When in `trigger` mode you need to set a specific `channel (CH)` from `1-255`


------

### Animations

| Key                             | Description                                                                  |
| :------------------------------ | :--------------------------------------------------------------------------- |
| **`F8`/`F9`**                   | Increase/Decrease the `milliseconds` of the current animation frame          |
| **`Ctrl` + `F8`/`F9`**          | **Fast** Increase/Decrease the `milliseconds`                                |
| **`Space`**                     | Play/Stop the animation                                                      |
| **`Ctrl`**                      | Pause the animation                                                          |
| **`Alt`**                       | Switch to **no CLS mode** *(the frames will draw on top of each other)*      |
| **`LShift`**                    | **Previous** frame                                                           |
| **`RShift`**                    | **Next** frame (if theres no next frame it adds a new one)                   |
| **`LCtrl` + `LShift`/`RShift`** | **Fast** frame scrolling                                                     |
| **`RCtrl` + `LShift`/`RShift`** | Scroll to **first**/**last** frame                                           |
| **`Delete`**                    | Delete the current frame                                                     |
| **`Insert`**                    | Lock/Unclock the frames so its **impossible** to add/delete a frame          |
| **`End`**                       | Add a frame next to the **current** frame                                    |
| **`F12`**                       | See the **"ghost"** of the **previous** frame *(recommended for animations)* |

------

### Sound effects

| Key                   | Description                                           |
| :-------------------- | :---------------------------------------------------- |
| **`LCtrl` + `Space`** | Add a **sound effect** *(accepts only `*.wav` files)* |
| **`Ctrl` + `Insert`** | Show the full path of the audio file                  |
| **`RCtrl` + `Space`** | Remove a directory from the beggining of the **path** |

### NOTE: Only `*.wav` files are accepted
