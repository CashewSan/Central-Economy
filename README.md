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
This mod overrides almost EVERY structure in ARMA Reforger to add a usage zone and item spawners to it. As well, it creates a new version of the Everon terrain to add tier zones for item spawning.

## USAGE
The spawn parameters of each item are configured through a Json in the server's profile folder. To get started with the mod, it goes as follows:
1. Download [CentralEconomy](https://reforger.armaplatform.com/workshop/6265238BFD2AC936-CentralEconomy) through the ARMA Workshop.
2. Do the usual for adding a mod to your server.
3. Load the **Everon-CentralEconomy.ent** world provided in CentralEconomy, create gamemode, and add the **CE_WorldValidationComponent** to your world's gamemode (very important).
  - If you have an existing world you want to add this mod to, you'll have to copy the tier zones from the Everon-CentralEconomy.ent world, or create your own tier zones. Either way, **CE_WorldValidationComponent** must exist in the gamemode!
4. Run the server ONCE to generate the default CE_ItemData.json file (file can be found in your server profile folder, and goes $profile/.CentralEconomy/CE_ItemData.json).
5. Configure your Json file to your liking or copy & paste the [default file](CE_ItemData.json), that I provided, into it and adjust where needed.
   - Parameters go as followed:
     - **Name** - Should match the item prefab name (Ex. the prefab "Smoke_M18_Yellow" should have the name "Smoke_M18_Yellow").
     - **Prefab** - Defines the prefab that should spawn. This is the **GUID** of the prefab, not the whole path.
     - **Rotation** - Defines the rotation of item when spawning, in XYZ format.
     - **Nominal** - Target item count across the entire map.
     - **Minimum** - Minimum item count across the entire map.
     - **Lifetime** - Length of time (in seconds) item can stay on the ground before despawning.
     - **Restock** - Length of time (in seconds) for item count to be added back to spawn queue.
     - **QuantityMax** - If applicable to item, the maximum quantity of internal stored item to be considered (Ex. ammo in a magazine, or jerrycan fuel amount).
     - **QuantityMin** - If applicable to item, the minimum quantity of internal stored item to be considered (Ex. ammo in a magazine, or jerrycan fuel amount).
     - **Category** - Category the item falls into, can only choose one. **Must be capitalized!** Currently includes:
       - CLOTHING
       - FOOD
       - TOOLS
       - WEAPONS
     - **Usages** - Type of areas the item can spawn into. This can be multiple or only one, but must maintain array format. **Must be capitalized!** Currently includes:
       - COAST
       - FARM
       - HUNTING
       - INDUSTRIAL
       - MEDIC
       - MILITARY
       - POLICE
       - TOWN
     - **Tiers** - Type of tiers the item can spawn into. This can be multiple or only one, but must maintain array format. **Must be capitalized!** Currently includes:
       - TIER1
       - TIER2
       - TIER3
       - TIER4
6. Save CE_ItemData.Json file
7. Launch server, and item spawns should populate!

### Things to Keep in Mind
- I *HIGHLY* recommend adjusting the nominal and minimum values for each item in the CE_ItemData.json, otherwise it'll be a poor experience as everything by default has an equal chance of spawning in their respective locations.
- The spawning system is currently set to run every 15 seconds on server launch to quickly populate item spawns that are empty, doing a single tier at a time. Then after the initial spawns are performed by the spawning system, it increases to check every 120 seconds (2 minutes).
- While the spawning system is performing it's initial run through of item spawns, it's **normal** to see minor stuttering of server performance. After the initial run through, the stuttering is virtually unnoticeable or nonexistent, even.
