# Stealth Game
A personal, independent immersive-sim prototype built to learn Unreal Engine 5.7 and C++ gameplay programming. This project demonstrates modular gameplay systems (player abilities, inventory, AI, quests, time/visibility mechanics) implemented in C++ — not studio work, but a technical sample of engine-level gameplay engineering.

### Note this is WiP project and new features and improvements are regularly added

## How To Start
- Clone the repository
- Fetch git LFS
- Start the project using Unreal Engine 5.7
- Start game in editor

## Controls
Mouse + W,S,A,D - Movement
Space - Jump
Ctrl - Crouch
Shift - Sprint
E/R - Primary/secondary interaction
TAB/I/J - Toggle menus

## Implemented Features Include:
- Player movement & abilities: sprint, crouch and stamina managed via Gameplay Ability System (GAS).
- Interactables: IInteractable-based interaction system and pickup items (see Source/Stealth/Private/Interactables).
- Inventory: InventoryComponent and InventoryManagerSubsystem — modular C++ inventory manager and item handling (Source/Stealth/Private/Inventory).
- NPC AI: StateTree-driven AI with perception (sight & hearing) and reactive behaviours (see Source/Stealth/Private/Characters and StateTree setup).
- Quests & daily tasks: quest conditions and objectives (Source/Stealth/Private/Quests and Source/StealthQuestSystem module).
- Time & visibility: Day/Night cycle with lighting influencing player visibility and NPC reactions (Source/Stealth/Private/TimeSystem, Exposure).
- UI / HUD: in-game HUD for player status, inventory and quest feedback (Source/Stealth/Private/UI).



https://github.com/user-attachments/assets/a5fb0a66-59d2-4fb4-b15f-2e8c2a2c7502

The video above shows the flow of adding an item to inventory, being granted a new ability lockpicking thanks to possesing the item and then dropping lockpicks. 
