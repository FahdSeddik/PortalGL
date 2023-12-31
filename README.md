<p align="center">

 <img src="Banner.png" alt="sentiments" width="45%" align="center"/>  
 <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/b7a81910-7a76-4b84-96da-68124a008818" align="left" width="27%"/>  
 <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/d347d4b4-9e34-4226-9f4f-8537d0aa9972" align="right" width="27%"/>  

</p>

<hr>

# Portal Recreation  

This is a recreation of the game Portal, a game developed by Valve. The game involves using a portal gun to create portals between two surfaces, allowing the player to traverse the environment and solve puzzles.

## Gameplay and Screenshots  
<details><summary><h3 align="left">🎮 Gameplay</summary>

https://github.com/FahdSeddik/PortalGL/assets/62207434/021f502b-ad35-4d57-9e31-c6afa539c0d3

</details>



<details><summary><h3 align="left">📷 Screenshots</summary>
<table> 
<tr><td> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/68b244f8-52b9-4afa-a833-663029603a3f"> </td><td> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/53819058-9e3f-4e41-90d9-da9a34a018bb"> </td></tr>
<tr><td> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/c8148c24-28f5-4d7a-9a1d-6ee9cd71ecc4"> </td><td> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/c8148c24-28f5-4d7a-9a1d-6ee9cd71ecc4"> </td></tr>
<tr><td> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/4f1ca49f-65cb-4ab8-b428-4b165d1a066c"> </td><td> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/6e584008-e14c-46fe-bea2-1f6bad7c2cf1"> </td></tr>
<tr><td> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/e47da21e-5599-45f1-80ca-c90ec39fa1ed"> </td><td> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/56af1a92-28e5-48cf-9322-926f2db5ed4a"> </td></tr>
</table> 
</details>  

## Contents  

- [Portal Recreation](#portal-recreation)
  - [Gameplay & Screenshots](#gameplay-and-screenshots)
  - [Contents](#contents)
  - [Installation](#installation)
  - [How to Run](#how-to-run)
  - [Controls](#controls)
  - [To Do](#to-do)

## Installation  

To install the game, you need to clone this repository to your local machine. You can use the following command in your terminal:
```
git clone https://github.com/FahdSeddik/PortalGL.git
```

## How to Run  

To run the game, you need to compile the source code using [CMake](https://cmake.org/download/) and the provided `CMakeLists.txt` file.  
```
mkdir build
cmake -B build -S .
cmake --build build
```
An alternative for running the above commands, is to do the following after installing [CMake](https://cmake.org/download/).  
> 1. Download CMake Extension for VSCode
> 2. Click Build
> 3. Launch using `./bin/GAME_APPLICATION.exe` (make sure CWD is root folder)

## Controls  
 
  
| Movement | Sprint & Jump | Shooting | Interaction | 
| :------: | :------: | :------: | :------: |
| <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/a6769d1e-66f9-4321-9675-fde592886950" align="center" width="30%"/> <br> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/bae0337c-2515-432c-8dbd-4499e18af57f" width="30%"/> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/db894041-c448-4898-81a3-d62d5421bdad" width="30%"/><img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/321e11bb-a6f4-4de1-9621-8223749e9bf1" width="30%"/> | <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/6085395b-2af8-4566-9678-bc992ac7a5cf" /> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/8929d03f-8118-40f2-8fff-bd3bb742d3a5" /> |<img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/af7c63f0-7063-4885-b6da-0e874f352ff2" /> <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/15ffccca-8e75-4110-8c05-cbe9bf065235" /> | <img src="https://github.com/FahdSeddik/PortalGL/assets/62207434/d7640d66-e8aa-4843-8a9f-6a5ca1c382e7"/> |



## To Do  

- [ ] Integrate OpenAL for audio
- [ ] Handle Object intersecting portals
- [ ] Implement level loading
- [ ] Player Character Rendering
- [ ] Change Start Menu Screen

