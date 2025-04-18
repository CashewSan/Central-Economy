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
This mod overrides almost every VANILLA structure in ARMA Reforger to add a usage zone and item spawners to it. As well, it creates a new version of the Everon terrain with added tier zones for item spawning.

## USAGE
The spawn parameters of each item are configured through a config in the server's profile folder. To get started with the mod, it goes as follows:
1. Download [CentralEconomy](https://reforger.armaplatform.com/workshop/6265238BFD2AC936-CentralEconomy) through the ARMA Workshop.
2. Do the usual for adding a mod to your server.
3. Load the **Everon-CentralEconomy.ent** world provided in CentralEconomy, create gamemode, and add the **CE_WorldValidationComponent** to your world's gamemode (very important).
  - If you have an existing world you want to add this mod to, you'll have to copy the tier zones from the Everon-CentralEconomy.ent world, or create your own tier zones. Either way, **CE_WorldValidationComponent** must exist in the gamemode!
4. Run the server ONCE to generate the CE_ItemData.conf file (file can be found in your server profile folder, and goes $profile/CentralEconomy/CE_ItemData.conf).
5. Configure your config file to your liking or copy & paste the [default file](CE_ItemData.conf), that I provided, into it and adjust where needed. **ADJUSTMENTS MUST BE MADE WITHIN WORKBENCH** (To do so, add the file to your Workbench profile folder, and open Central Economy in Workbench to edit it)
   - Parameters go as followed:
     - **Name** - Should match the item prefab name (Ex. the prefab "Smoke_M18_Yellow" should have the name "Smoke_M18_Yellow"). **MUST BE UNIQUE!!**
     - **Prefab** - Defines the prefab that should spawn.
     - **Rotation** - Defines the rotation of item when spawning, in XYZ format.
     - **Nominal** - Target item count across the entire map.
     - **Minimum** - Minimum item count across the entire map.
     - **Lifetime** - Length of time (in seconds) item can stay on the ground before despawning.
     - **Restock** - Length of time (in seconds) for item count to be added back to spawn queue.
     - **QuantityMax** - If applicable to item, the maximum quantity (as a percentage) of internal stored item to be considered (Ex. ammo in a magazine, or jerrycan fuel amount).
     - **QuantityMin** - If applicable to item, the minimum quantity (as a percentage) of internal stored item to be considered (Ex. ammo in a magazine, or jerrycan fuel amount).
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
6. Save CE_ItemData.conf file
7. Launch server, and item spawns should populate over time!

### Things to Keep in Mind
- On Everon, there are roughly 4600 spawn points. This means for a good item spawning experience, you'll want more than that (~4600) total combined of the item nominals, preferably way more so the spawners have more variety to choose from. So keep that in mind with the second note here below.
- I **HIGHLY** recommend adjusting the nominal and minimum values for each item in the CE_ItemData.conf, otherwise it'll be a poor experience as everything by default has an equal chance of spawning in their respective locations.
- While the spawning system is performing, you shouldn't really see *too* terribly much of a performance hit (from my testing), but if you do experience too much performance degradation, please report it!
- If spawning is too slow, you can lower the spawn rate interval in your world's **CE_WorldValidationComponent**. By default, it's set to spawn an item every 0.5 seconds, lowering it will result in items spawning faster, but at a cost of performance!

## PLANNED FEATURES
- Add default Arland map tiers.
- Add ability for Dynamic Event loot spawns.
- Add ability to check for items inside storages.
- Further bug fixes and optimizations.

## KNOWN ISSUES
- Some items that spawn may be invisible to the player (it's actually just clipping into the structure, will fix as it's reported).
- Some spawns won't spawn items (this is due to the lack of diversity in the CE_ItemData.conf, not many vanilla items in the game tbh).
- Some items are very commonly spawning (this is also due to the lack of diversity in the CE_ItemData.conf, not many vanilla items in the game tbh. But also, double-check your nominal and minimum numbers in comparison to comparable items).
- Logs post "RESOURCES (E): Wrong GUID/name for resource @"{0000000000000000}$profile:/CentralEconomy/CE_ItemData.conf" in property "m_sDb"" (It's safe to ignore)
- Configs made before the major rewrite show blank (Open it in a text editor, like Notepad, and change CE_LootWhatever on the first line to **CE_ItemDataConfig**, save the file, then reload it into workbench)

**IF YOU EXPERIENCE ANY BUGS OR ISSUES, PLEASE REPORT THEM IN [ISSUES](https://github.com/CashewSan/Central-Economy/issues) HERE ON GITHUB OR DIRECTLY TO ME IN THE [ARMA DISCORD](https://discord.com/channels/105462288051380224/1301291009635909664) @CASHEWSAN**
