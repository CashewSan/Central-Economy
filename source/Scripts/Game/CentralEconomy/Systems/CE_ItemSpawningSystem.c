class CE_ItemSpawningSystem : GameSystem
{
	// To preface, this is a mess, but I'll try my best to keep comments for those reading lol
	
	protected ref array<ref CE_ItemData>				m_aItems 					= new array<ref CE_ItemData>; 					// storage of initial item data from config
	protected ref array<ref CE_Item>					m_aItemsToSpawn 				= new array<ref CE_Item>; 						// item data containing items that need to spawn
	protected ref array<ref CE_ItemData>				m_aSpawnedItems 				= new array<ref CE_ItemData>; 					// items that have spawned in the world
	protected ref array<CE_ItemSpawningComponent> 		m_aComponents 				= new array<CE_ItemSpawningComponent>; 			// initial pull of ALL item spawning components in the world
	protected ref array<ref CE_Spawn> 				m_aComponentsForSpawn 		= new array<ref CE_Spawn>;  						// item spawning components in the world WITH NO LOOT
	protected ref array<CE_Spawn> 					m_aMatchedSpawns 			= new array<CE_Spawn>;							// spawns matched to item array
	protected ref array<ref CE_ItemData>				m_aItemsNotRestocked 			= new array<ref CE_ItemData>;						// items whose restocking timer has not ended
	
	protected CE_WorldValidationComponent 				m_WorldValidationComponent;													// component added to world's gamemode for verification

	protected ref CE_ItemDataConfig					m_Config;																	// config containing item data (typically in server profile folder)
	protected CE_ELootTier							m_currentSpawnTier;															// current spawn tier that items are spawning at
	
	protected ref RandomGenerator 					m_randomGen 					= new RandomGenerator();							// vanilla random generator
	
	protected bool 									m_bHasGeneratedSpawnData		= false;											// has spawn data been generated?
	protected bool 									m_bHasGeneratedItemData		= false;											// has item data been generated?
	protected bool 									m_bWorldProcessed			= false;											// has the world been processed?
	
	protected float 									m_fTimer;																	// timer for check interval
	
	protected int									CHECK_INTERVAL; 																// how often the item spawning system will run (in seconds)
	
	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{
		if (m_aComponents.IsEmpty())
			Enable(false);
		
		CHECK_INTERVAL = 15;
		
		//DelayedInit();
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;

		if (m_fTimer < CHECK_INTERVAL)
			return;

		m_fTimer = 0;
		
		/*
		if (m_bWorldProcessed == true)
			ProcessData();
		else
			GetGame().GetCallqueue().CallLater(DelayedInit, 1000, false);
		*/
	}
	
	// GameSystem stuff
	//------------------------------------------------------------------------------------------------
	static CE_ItemSpawningSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
	}

	//------------------------------------------------------------------------------------------------
	void Register(notnull CE_ItemSpawningComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aComponents.Contains(component))
			m_aComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(notnull CE_ItemSpawningComponent component)
	{
		m_aComponents.RemoveItem(component);
		
		Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ItemData GetItem(CE_ItemDataConfig config, CE_ELootTier tier, CE_ELootUsage usage)
	{
		CE_ItemData itemChosen;
		
		array<CE_Item> items = GenerateItems(ValidateItemData(config.m_ItemData), tier, usage);
		
		if (items.IsEmpty())
			itemChosen = null;
		else
			itemChosen = SelectItem(items);
		
		return itemChosen;
	}
	
	//------------------------------------------------------------------------------------------------
	protected array<CE_ItemData> ValidateItemData(array<ref CE_ItemData> items)
	{
		array<CE_ItemData> itemData = {};
		
		if (!items || items && items.IsEmpty())
			return null;
		
		foreach(CE_ItemData item: m_Config.m_ItemData)
		{
			// basically, if missing any variable besides rotation or quantity min and max
			if(!item.m_sName || !item.m_sPrefab || !item.m_iNominal || !item.m_iMinimum || !item.m_iLifetime || !item.m_iRestock || !item.m_ItemCategory || !item.m_ItemUsages || !item.m_ItemTiers)
			{
				Print("[CentralEconomy] " + item.m_sName + " is missing information!", LogLevel.ERROR);
				continue;
			}
			
			itemData.Insert(item);
		}
		
		return itemData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected array<CE_Item> GenerateItems(array<CE_ItemData> items, CE_ELootTier tier, CE_ELootUsage usage)
	{
		array<CE_Item> itemsToSpawn = {};
		
		foreach (CE_ItemData itemData : items)
		{
			// matches tier & usage of spawner component, as well as current tier being processed
			if (itemData.m_ItemTiers & tier && itemData.m_ItemUsages & usage && itemData.m_ItemTiers & GetCurrentSpawnTier())
			{
				int itemTargetCount = DetermineItemTargetCount(itemData, itemData.m_iMinimum, itemData.m_iNominal);
				
				CE_Item item = new CE_Item(itemData, itemData.m_ItemTiers, itemData.m_ItemUsages, itemData.m_ItemCategory);
				
				if (!itemsToSpawn.Contains(item))
				{
					for (int i = 0; i < Math.ClampInt(itemTargetCount, 0, itemData.m_iNominal); i++)
					{
						m_aItemsToSpawn.Insert(item);
					}
				}
			}
		}
		
		return itemsToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int DetermineItemTargetCount(CE_ItemData item, int minimum, int nominal)
	{
		int targetCount = nominal;
		
		return targetCount;
	}
	
	//------------------------------------------------------------------------------------------------
	protected CE_ItemData SelectItem(array<CE_Item> itemArray)
	{
		if(itemArray.IsEmpty())
			return null;
		
		CE_ItemData itemData;
		
		map<CE_Item, int> counts = new map<CE_Item, int>;
		
		foreach (CE_Item item : itemArray)
		{
			if (counts.Contains(item))
			{
				counts[item] = counts[item] + 1;
			}
			else
				counts[item] = 1;
		}
		
		return itemData;
    }
	
	//------------------------------------------------------------------------------------------------
	protected bool Probability(float probability, float probabilityTotal)
	{
		int random = m_randomGen.RandInt(0, probabilityTotal);
		
		return random <= probability;
	}
	
	
	
	
	
	
	// Getters, Setters
	//------------------------------------------------------------------------------------------------
	CE_ELootTier GetCurrentSpawnTier()
	{
		if (!m_currentSpawnTier)
		{
			GetNextSpawnTier();
		}
		
		return m_currentSpawnTier;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCurrentSpawnTier(CE_ELootTier tier)
	{
		m_currentSpawnTier = tier;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetNextSpawnTier()
	{	
		switch (GetCurrentSpawnTier())
		{
			case null:
				SetCurrentSpawnTier(CE_ELootTier.TIER1);
				break;
		
			case CE_ELootTier.TIER1:
				GetCurrentSpawnTier() = CE_ELootTier.TIER2;
				break;
			
			case CE_ELootTier.TIER2:
				GetCurrentSpawnTier() = CE_ELootTier.TIER3;
				break;
			
			case CE_ELootTier.TIER3:
				GetCurrentSpawnTier() = CE_ELootTier.TIER4;
				break;
			
			case CE_ELootTier.TIER4:
				GetCurrentSpawnTier() = CE_ELootTier.TIER1;
				CHECK_INTERVAL = 120;
				break;
		}
		
		//Print(" " + GetCurrentSpawnTier() + " " + CHECK_INTERVAL);
	}
}





class CE_Item
{
	CE_ItemData Item;
	CE_ELootTier Tiers;
	CE_ELootUsage Usages;
	CE_ELootCategory Category;
	
	//------------------------------------------------------------------------------------------------
	void CE_Item(CE_ItemData item, CE_ELootTier tiers, CE_ELootUsage usages, CE_ELootCategory category)
	{
		Item = item;
		Tiers = tiers;
		Usages = usages;
		Category = category;
	}
}

class CE_Spawn
{
	CE_ItemSpawningComponent Spawn;
	CE_ELootTier Tier;
	CE_ELootUsage Usage;
	CE_ELootCategory Category;
	ref array<CE_Item> Items = {};
	
	//------------------------------------------------------------------------------------------------
	void CE_Spawn(CE_ItemSpawningComponent spawn, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		Spawn = spawn;
		Tier = tier;
		Usage = usage;
		Category = category;
		Items = new array<CE_Item>;
	}
}