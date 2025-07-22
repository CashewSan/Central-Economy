<div align="center">
<picture>
  <source media="(prefers-color-scheme: dark)" width="400" srcset="https://github.com/CashewSan/Central-Economy/blob/main/.github/CE_Logo_Dark.png?raw=true">
  <source media="(prefers-color-scheme: light)" width="400" srcset="https://github.com/CashewSan/Central-Economy/blob/main/.github/CE_Logo_Light.png?raw=true">
  <img alt="Central Economy" width="400" src="https://github.com/CashewSan/Central-Economy/blob/main/.github/CE_Logo_Light.png?raw=true">
</picture>
<br/><br/>

<div align="left">
  
## SUMMARY
Central Economy allows for the dynamic spawning of items around a terrain with a master config file!
This mod overrides almost every VANILLA structure in ARMA Reforger to add a usage zone and item spawners to it, as well, it overrides a large amount of furniture prefabs to allow the ability to be searched. It currently includes a version of the Everon terrain with added tier zones for item spawning.

## USAGE
The spawn parameters of each item are configured through a config in the server's profile folder. To get started with the mod, it goes as follows:
1. Download [CentralEconomy](https://reforger.armaplatform.com/workshop/6265238BFD2AC936-CentralEconomy) through the ARMA Workshop.
2. Do the usual for adding a mod to your server.
3. Load the **Everon-CentralEconomy.ent** world provided in CentralEconomy in Workbench, create your gamemode setup, and add the **CE_WorldValidationComponent** to your world's gamemode entity (very important).
  - If you have an existing world you want to add this mod to, you'll have to copy the tier zones from the Everon-CentralEconomy.ent world, or create your own tier zones. Either way, **CE_WorldValidationComponent** must exist in the gamemode!
  - CE_WorldValidationComponent parameters can be adjusted, look [here](https://github.com/CashewSan/Central-Economy?tab=readme-ov-file#ce_worldvalidationcomponent-attribute-parameters) for info on how to do so.
4. Run the server ONCE to generate the CE_ItemData.conf file (file can be found in your server profile folder, and goes $profile/CentralEconomy/CE_ItemData.conf).
5. Configure your config file to your liking or copy & paste the [default file](CE_ItemData.conf), that I provided, into it and adjust where needed. **ADJUSTMENTS MUST BE MADE WITHIN WORKBENCH** (To do so, add the file to your Workbench profile folder, and open Central Economy in Workbench to edit it)
   - Parameters go as followed:
     - **Name** - Should typically match the item prefab name (Ex. the prefab "Smoke_M18_Yellow" should have the name "Smoke_M18_Yellow"). **MUST BE UNIQUE!!**
     - **Prefab** - Defines the prefab that should spawn.
     - **Rotation** - Defines the rotation of item when spawning, in XYZ format.
     - **Nominal** - Target item count across the entire map.
     - **Minimum** - Minimum item count across the entire map.
     - **Lifetime** - Length of time (in seconds) item can stay on the ground before despawning.
     - **Restock** - Length of time (in seconds) for item count to be added back to spawn queue.
     - **QuantityMax** - If applicable to item, the maximum quantity (as a percentage) of internal stored item to be considered (Ex. ammo in a magazine, or jerrycan/vehicle fuel amount).
     - **QuantityMin** - If applicable to item, the minimum quantity (as a percentage) of internal stored item to be considered (Ex. ammo in a magazine, or jerrycan/vehicle fuel amount).
     - **Category** - Category the item falls into, can only choose one. Currently includes:
       - CLOTHING
       - FOOD
       - TOOLS
       - WEAPONS
       - VEHICLES
       - VEHICLES_SEA
       - VEHICLES_AIR
     - **Usages** - Type of areas the item can spawn into. This can be multiple or only one. Currently includes:
       - COAST
       - FARM
       - HUNTING
       - INDUSTRIAL
       - MEDIC
       - MILITARY
       - POLICE
       - TOWN
     - **Tiers** - Type of tiers the item can spawn into. This can be multiple or only one. Currently includes:
       - TIER1
       - TIER2
       - TIER3
       - TIER4
6. Save CE_ItemData.conf file and copy updated version back to your server profile folder
7. Launch server!

### CE_WorldValidationComponent Attribute Parameters
The CE_WorldValidationComponent has 3 attribute parameters that you can adjust in your gamemode entity to change how your item spawning system performs.
- Item Spawning Frequency
  - Determines how often an item will queue for spawning after the initial spawning phase is performed, it's based on seconds. So if set to 30, it'll queue the spawn of an item every 30 seconds.
  - **LOWER VALUES ARE HARDER ON PERFORMANCE**
  - Default value is 30 seconds
- Item Spawning Ratio
  - Ratio of items the system to try to spawn compared to spawners in the world (not including searchable containers). Set as a 0 - 1 percentage, so if set as 0.25, only %25 of the total spawner count (~4750 spawners) will have an item spawned on it at a time.
  - **HIGHER VALUES ARE HARDER ON PERFORMANCE**
  - Default value is 0.25 (%25)
- Searchable Container Chance
  - Chance that a searchable container entity will actually be searchable. Set as a 0 - 1 percentage, so if set as 0.5, a container entity has a %50 chance upon initialization to be searchable.
  - Higher values *shouldn't* effect performance too much, but it will cause more searchable container entities to be available in the world and need to be processed, so take that as you will.
  - Default value is 0.25 (%25)
  - Whether a container entity is searchable or not is determined randomly upon each server start. So one container entity that was searchable may not be searchable the next restart.

### Things to Keep in Mind
- On Everon, there are roughly 5000 spawn points. From testing, this means if you want ALL spawners to have a item spawned at them, you'll want about 10x that spawner count in the combined total of item nominal values.
  - I **DON'T** recommend to have ALL spawners be able to spawn an item (having item spawning ratio set to 1) and then additionally having that many item nominals. It's heavy on performance.
  - I **DO** recommend having a total item nominal value comparable to the spawner count multiplied by your item spawning ratio. So if you have a 0.25 item spawning ratio, ~5000 multiplied by 0.25 is ~1250 spawners, so if 10x'ing that value, you'd want about a ~12500 total value of item nominals.
  - This is all theoretical from my hours of testing. If you find you want a less total item nominal, then go for it. I'm just saying how I saw it go smoothly with running around and finding loot and searching containers. With a low total item nominal, I saw that there wasn't enough items to be processed.
    - This of course could be remedied with setting high nominal counts to low tier items, and then setting low nominal counts to high tier items. But for default showcase purposes, this is how I have it setup as plug and play.
- If running [Enfusion Persistence Framework](https://reforger.armaplatform.com/workshop/5D6EBC81EB1842EF-EnfusionPersistenceFramework) additionally with Central Economy:
  - You should additionally run the [CentralEconomy EPF Compatibility mod](https://reforger.armaplatform.com/workshop/64601621C028A690-CentralEconomy-EPF) to have intended behavior. With running it, for items spawned by the system, if they're still on a spawner as of last server shutdown, they will not persist to next server start.
    - This is to help with performance with persistence, as loading hundreds and setting all their variables to match last session is performance intensive. Maybe it'll change with vanilla persistence when that finally gets introduced.
  - Not running the CentralEconomy EPF Compatibility mod will cause unintended behaviors when also running Enfusion Persistence Framework.
- For a better experience, you **WILL** need to adjust the nominal and minimum values for each item in the CE_ItemData.conf, otherwise it'll be a poor experience as everything by default has an equal amount of minimum and nominal values (for showcasing purposes).
  - I don't provide a universal best-experience-possible config by default because it absolutely depends on what *you* are looking for in loot spawning. So that's why, for the most part, I leave it up to you. I've just done most of the work already.

## HOW IT WORKS
###  Here's how CentralEconomy works:
  - Upon server initilization, Usage and Tier Trigger Areas will query for each spawner and searchable container within them and set their respective usage and tier values.
  - The CE_ItemSpawningSystem (the brains of CentralEconomy), upon being enabled via script, starts processing the CE_ItemData.conf's CE_ItemData into CE_Items (custom class to contain all necessary item attributes for spawning).
    - Upon this point, if you have errors with an item in your config, error logs will start to appear stating it.
  - Once the trigger areas are done querying and setting variables, a script invoker is called to the CE_ItemSpawningSystem to then activate the Initial Spawning Phase.
    - The Initial Spawning Phase does a MASS item spawning to a specific spawner count (based on the Item Spawning Ratio set in CE_WorldValidationComponent). Spawners are randomly selected, and then items are chosen based on what's available and if they match the selected spawner's attributes.
  - Upon the finish of the Initial Spawning Phase, periodic spawning starts.
    - Periodic spawning will ONLY happen if the spawned item count drops below the total spawner count multiplied by the item spawning ratio set in CE_WorldValidationComponent.
    - With periodic spawning, if there is an available spawner for an item, then the item is queued to be spawned.

###  (For items spawned to a spawner and NOT a searchable container action)
  - Once an item spawns, it is given a timer (it's lifetime) to be either despawned or taken from the spawner. Additionally, the CE_Item's available count is depleted by one each time it's respective item spawns.
    - If it's lifetime timer reaches 0, it is despawned.
    - If it is taken, it's lifetime timer stops and then it's restock timer starts.
      - Upon the restock timer reaching 0, a single item count is added to the respective CE_Item's available count.
  - Once an item is taken from the spawner, the spawner reset timer starts.
    - Once the spawner reset timer reaches 0, the spawner is available to have an item be spawned on it again.
    
###  (For items spawned to a searchable container action)
  - Once an item spawns, instead of relying on the item's lifetime to be despawned, it relies on the searchable container's reset time (set to 3600 seconds by defaul, or 1 hour). Additionally, the CE_Item's available count is depleted by one each time it's respective item spawns.
    - When the searchable container's reset time reaches 0, all the items within the container storage (if any are still within it) are despawned and the container becomes searchable once more.
    - When an item is taken, the item's restock timer starts.
      - Upon the restock timer reaching 0, a single item count is added to the respective CE_Item's available count.

## PLANNED FEATURES
- Add default Arland map tiers.
- Add ability for Dynamic Event loot spawns.
- Further bug fixes and optimizations.

## TROUBLESHOOTING
- Some items that spawn may be invisible to the player.
  - It's actually just clipping into the structure, will fix as it's reported.
- Some spawners won't spawn items.
  - This is due to the lack of diversity in the CE_ItemData.conf, typically due to there not being many vanilla items in the game.
- Some items are very commonly spawning.
  - This is also due to the lack of diversity in the CE_ItemData.conf. But also, double-check your nominal and minimum numbers compared to comparable items.
- Logs post "RESOURCES (E): Wrong GUID/name for resource @"{0000000000000000}$profile:/CentralEconomy/CE_ItemData.conf" in property "m_sDb"" upon server boot or Workbench load.
  - Safe to ignore, means nothing as long as the CE_ItemData.conf is loading fine. If the CE_ItemData.conf is not loading fine, you'll typically see other errors in logs stating it.
- Configs made before the original major rewrite show blank.
  - Open it in a text editor, like Notepad, and change CE_LootSpawningConfig on the first line to **CE_ItemDataConfig**, save the file, then reload it into workbench.

**IF YOU EXPERIENCE ANY BUGS OR ISSUES, PLEASE REPORT THEM IN [ISSUES](https://github.com/CashewSan/Central-Economy/issues) HERE ON GITHUB OR DIRECTLY TO ME IN THE [ARMA DISCORD](https://discord.com/channels/105462288051380224/1301291009635909664) @CASHEWSAN**
