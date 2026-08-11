# Stealth Game
A personal, independent immersive-sim prototype built to learn Unreal Engine 5.7 and C++ gameplay programming. This project demonstrates modular gameplay systems (player abilities, inventory, AI, quests, time/visibility mechanics) implemented in C++ — not studio work, but a technical sample of engine-level gameplay engineering.

## How To Start
- Clone the repository
- Fetch git LFS
- Start the project using Unreal Engine 5.7
- Start game in editor

## Implemented Features Include:
- Player movement & abilities: sprint, crouch and stamina managed via Gameplay Ability System (GAS).
- Interactables: IInteractable-based interaction system and pickup items (see Source/Stealth/Private/Interactables).
- Inventory: InventoryComponent and InventoryManagerSubsystem — modular C++ inventory manager and item handling (Source/Stealth/Private/Inventory).
- NPC AI: StateTree-driven AI with perception (sight & hearing) and reactive behaviours (see Source/Stealth/Private/Characters and StateTree setup).
- Quests & daily tasks: quest conditions and objectives (Source/Stealth/Private/Quests and Source/StealthQuestSystem module).
- Time & visibility: Day/Night cycle with lighting influencing player visibility and NPC reactions (Source/Stealth/Private/TimeSystem, Exposure).
- UI / HUD: in-game HUD for player status, inventory and quest feedback (Source/Stealth/Private/UI).
