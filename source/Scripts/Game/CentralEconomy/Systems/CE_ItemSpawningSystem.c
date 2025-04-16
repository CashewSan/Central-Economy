class CE_ItemSpawningSystem : GameSystem
{
	// To preface, this is a mess, but I'll try my best to keep comments for those reading lol
	
	protected ref array<ref CE_ItemData>				m_aSpawnedItems 				= new array<ref CE_ItemData>; 					// items that have spawned in the world
	protected ref array<CE_ItemSpawningComponent> 		m_aComponents 				= new array<CE_ItemSpawningComponent>; 			// initial pull of ALL item spawning components in the world
	protected ref array<CE_ItemSpawningComponent> 		m_aComponentsWithoutItem 		= new array<CE_ItemSpawningComponent>; 			// copy of m_aComponents, with components removed over time when they have items spawned
	protected ref map<ref CE_ItemData, string>			m_aItemsNotRestockReady 		= new map<ref CE_ItemData, string>;				// items whose restocking timer has not ended
	
	protected CE_WorldValidationComponent 				m_WorldValidationComponent;													// component added to world's gamemode for verification
	protected ref RandomGenerator 					m_randomGen 					= new RandomGenerator();							// vanilla random generator

	protected bool 									m_bWorldProcessed			= false;											// has the world been processed?
	
	protected float 									m_fTimer						= 0;												// timer for spawning check interval
	protected float 									m_fSpawnerResetTimer			= 0;												// timer for spawner reset
	protected float 									m_fStallTimer				= 0;												// timer for stall check interval
	protected float									m_fCheckInterval				= 0; 											// how often the item spawning system will run (in seconds)
	protected const float								m_fSpawnerResetCheckInterval	= 5;												// how often the system will check for spawner reset
	protected const float								m_fStallCheckTime			= 10; 											// how long the system can stall before lowering the check interval before conditions are met in OnUpdate() (Set to 10 seconds)
	
	protected int									m_iSpawnedItemCount			= 0;												// count for spawned items, used to track when check interval changes
	
	
	//------------------------------------------------------------------------------------------------
	//! Copies m_aComponents array to m_aComponentsWithoutItem, sets m_fCheckInterval, and calls DelayedInit()
	override void OnInit()
	{
		if (m_aComponents.IsEmpty())
		{
			Enable(false);
		}
		else if (m_aComponentsWithoutItem.IsEmpty())
		{
			m_aComponentsWithoutItem.Copy(m_aComponents);
		}
		
		DelayedInit();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Tick method, currently set to start spawning process of an item every 0.1 seconds, then adjust to 2 seconds once all spawner components have been cycled
	override event protected void OnUpdate(ESystemPoint point)
	{
		if (m_fCheckInterval == 0)
			return;
		
		float timeSlice = GetWorld().GetFixedTimeSlice();
		
		m_fTimer += timeSlice;
		m_fStallTimer += timeSlice;
		m_fSpawnerResetTimer += timeSlice;
		
		if (m_fTimer >= m_fCheckInterval)
		{
			m_fTimer = 0;
			
			if (m_bWorldProcessed && !m_aComponents.IsEmpty())
			{
				SelectSpawnerAndItem();
				
				//Print("Spawned Item Count: " + m_iSpawnedItemCount);
				//Print("Components Without Item Count: " + (m_aComponentsWithoutItem.Count() - 1));
				
				/*
				if (m_iSpawnedItemCount >= m_aComponents.Count() && m_fCheckInterval != 1)
				{
					//Print("Check interval set to 1 second!");
					
					m_fCheckInterval = 1;
				}
				*/
			}
			else
				GetGame().GetCallqueue().CallLater(DelayedInit, 100, false);
		}
		
		if (m_fStallTimer >= m_fStallCheckTime && m_fCheckInterval != 1.0)
		{
			m_fCheckInterval = 1;
			ResetStallTimer();
		}
		
		if (m_fSpawnerResetTimer >= m_fSpawnerResetCheckInterval)
		{
			m_fSpawnerResetTimer = 0;
			
			foreach (CE_ItemSpawningComponent comp : m_aComponents)
			{
				comp.Update(m_fSpawnerResetCheckInterval);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Delayed initialization, sets the m_WorldValidationComponent, sets m_WorldProcessed as true if found, and starts the process of spawning
	protected void DelayedInit()
	{
		if(GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
		
		if(m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.HasWorldProcessed() && !m_aComponents.IsEmpty())
			{	
				Print("World processed!");
				
				m_bWorldProcessed = true;
				
				if (m_WorldValidationComponent.m_iItemSpawningRate)
					m_fCheckInterval = m_WorldValidationComponent.m_iItemSpawningRate;
				else
					m_fCheckInterval = 0.5;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Selects a randomized spawner component to try to spawn an item at, and requests the randomized item to be spawned
	protected void SelectSpawnerAndItem()
	{
		int index;
		
		if (m_aComponentsWithoutItem.Count() > 1)
			index = m_randomGen.RandInt(0, m_aComponentsWithoutItem.Count());
		else if (m_aComponentsWithoutItem.Count() == 1)
			index = 0;
		else
			return;
		
		if (m_aComponentsWithoutItem.IsEmpty() || m_aComponentsWithoutItem[index].HasItemSpawned())
			return;
		else
		{
			CE_ItemData item = m_aComponentsWithoutItem[index].RequestItemToSpawn();
			
			if (item)
				TryToSpawnItem(m_aComponentsWithoutItem[index], item);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! requests to item to be spawned via the spawner component
	protected void TryToSpawnItem(CE_ItemSpawningComponent comp, CE_ItemData item)
	{
		comp.TryToSpawnItem(item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calls to generate items and calls to get a selected item
	CE_ItemData GetItem(array<ref CE_ItemData> itemDataArray, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		if (itemDataArray.IsEmpty())
		{
			return null;
		}
		
		array<ref CE_Item> items = GenerateItems(itemDataArray, tier, usage, category);
		
		if (items.IsEmpty())
			return null;
		else
			return SelectItem(items);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Generates items and separates item data that has necessary information and corresponds to spawner component attributes
	protected array<ref CE_Item> GenerateItems(array<ref CE_ItemData> items, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		array<ref CE_Item> itemsToSpawn = {};
		
		foreach (CE_ItemData itemData : items)
		{
			// basically, if missing any variable besides rotation or quantity min and max
			if(!itemData.m_sName 
			|| !itemData.m_sPrefab 
			|| !itemData.m_iNominal 
			|| !itemData.m_iMinimum 
			|| !itemData.m_iLifetime 
			|| !itemData.m_iRestock 
			|| !itemData.m_ItemCategory 
			|| !itemData.m_ItemUsages 
			|| !itemData.m_ItemTiers)
			{
				Print("[CentralEconomy] " + itemData.m_sName + " is missing information!", LogLevel.ERROR);
				continue;
			}
			else if (itemData.m_ItemTiers & tier 
			&& itemData.m_ItemUsages & usage 
			&& itemData.m_ItemCategory & category) // has matching tier, usage, and category with spawner component
			{
				int itemTargetCount = DetermineItemTargetCount(itemData, itemData.m_iMinimum, itemData.m_iNominal);
				
				//Print("Item: " + itemData.m_sName + ", TargetCount: " + itemTargetCount);
				
				if (itemTargetCount != 0)
				{
					CE_Item item = new CE_Item(itemData, itemData.m_ItemTiers, itemData.m_ItemUsages, itemData.m_ItemCategory);
					
					if (!itemsToSpawn.Contains(item))
					{
						for (int i = 0; i < Math.ClampInt(itemTargetCount, 0, itemData.m_iNominal); i++)
						{
							itemsToSpawn.Insert(item);
						}
					}
				}
			}
		}
		
		return itemsToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Determines the item count of based of nominal, minimum, already spawned items, and those not restock ready
	protected int DetermineItemTargetCount(CE_ItemData item, int minimum, int nominal)
	{
		map<ref CE_ItemData, string> itemsNotRestockReady = GetItemsNotRestockReady();
		array<ref CE_ItemData> spawnedItems = GetSpawnedItems();
		
		int itemCount = 0;
		
		// for every item spawned in the world
		foreach (CE_ItemData spawnedItem : spawnedItems)
		{
			string name = spawnedItem.m_sName;
			
			// increase item count if there is a name match
			if (name == item.m_sName)
			{
				itemCount = itemCount + 1;
			}
		}
		
		int targetCount = Math.ClampInt(m_randomGen.RandIntInclusive(minimum, nominal) - itemCount, 0, nominal);
		
		for (int i = 0; i <= itemsNotRestockReady.Count(); i++)
		{
			if (itemsNotRestockReady.GetKeyByValue(item.m_sName))
				targetCount = Math.ClampInt(targetCount - 1, 0, nominal);
		}
		
		//Print("Item: " + item.m_sName + "MaxTargetCount: " + targetCount);
		
		//int minTargetCount = Math.ClampInt(minimum - itemCount, 0, nominal);
		
		//Print("Item: " + item.m_sName + ", TargetCount: " + Math.ClampInt(targetCount, 0, nominal));
		
		return Math.ClampInt(targetCount, 0, nominal);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Selects item from itemArray, chooses it based on probability algorithm
	protected CE_ItemData SelectItem(array<ref CE_Item> itemArray)
	{
		if(itemArray.IsEmpty())
			return null;
		
		CE_ItemData itemData;
		
		//Print(itemArray.Count());
		
		map<CE_Item, int> counts = new map<CE_Item, int>;
		
		int probabilityTotal = 0;
		
		for (int i = 0; i < itemArray.Count(); i++)
		{
			if (itemArray.Contains(itemArray[i]))
			{
				counts[itemArray[i]] = counts[itemArray[i]] + 1;
			}
			else
			{	
				counts[itemArray[i]] = 1;
			}
			
			probabilityTotal += counts.Get(itemArray[i]);
		}
		
		foreach (CE_Item item : itemArray)
		{
			if (Probability(counts.Get(item), probabilityTotal))
			{
				itemData = item.Item;
				break;
			}
		}
		
		//Print("Probability Total: " + probabilityTotal);
		
		return itemData;
    }
	
	//------------------------------------------------------------------------------------------------
	//! Chooses a randomized number between 0 and the probabilityTotal to the select and item it it's value is greater than said randomized number
	protected bool Probability(float probability, float probabilityTotal)
	{
		//Print("Probability: " + probability);
		
		int random = m_randomGen.RandInt(0, probabilityTotal);
		
		return probability >= random;
	}
	
	// GameSystem stuff
	//------------------------------------------------------------------------------------------------
	//! Gets the system instance
	static CE_ItemSpawningSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
	}

	//------------------------------------------------------------------------------------------------
	//! Registers component
	void Register(notnull CE_ItemSpawningComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aComponents.Contains(component))
			m_aComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	//! Unregisters component
	void Unregister(notnull CE_ItemSpawningComponent component)
	{
		m_aComponents.RemoveItem(component);
		
		Enable(false);
	}
	
	// Getters, Setters
	//------------------------------------------------------------------------------------------------
	//! Gets spawned item array
	array<ref CE_ItemData> GetSpawnedItems()
	{
		return m_aSpawnedItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets spawner array, but only of spawners without an item
	array<CE_ItemSpawningComponent> GetComponentsWithoutItem()
	{
		return m_aComponentsWithoutItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the array for items that are NOT restock ready
	map<ref CE_ItemData, string> GetItemsNotRestockReady()
	{
		return m_aItemsNotRestockReady;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets the spawn item count
	int GetSpawnedItemCount()
	{
		return m_iSpawnedItemCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the spawn item count
	void SetSpawnedItemCount(int count)
	{
		m_iSpawnedItemCount = count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resets stall timer
	void ResetStallTimer()
	{
		m_fStallTimer = 0;
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

/*
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
*/