/*
[EntityEditorProps(category: "CentralEconomy/Entities", description: "")]
class CE_LootSpawningClass : GenericEntityClass
{
}

class CE_LootSpawning : GenericEntity
{
	[Attribute(ResourceName.Empty, UIWidgets.Object, "Item data config to be used", "conf", category: "Item Data")]
	ref array<ref ResourceName> m_sConfigs;
	
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Category of loot spawn", enums: ParamEnumArray.FromEnum(CE_ELootCategory), category: "Item Data")]
	CE_ELootCategory m_Category;
	
	ref array<ref CE_ItemData>			m_aItems 					= new array<ref CE_ItemData>;
	ref array<CE_ELootCategory> 			m_aCategories 				= new array<CE_ELootCategory>;
	ref array<CE_ELootUsage> 				m_aUsages 					= new array<CE_ELootUsage>;
	ref array<CE_ELootTier> 				m_aTiers 					= new array<CE_ELootTier>;
	
	ref array<CE_LootSpawning> 			m_aSpawns 					= new array<CE_LootSpawning>;
	
	ref array<IEntity> 					spawnEntities 				= new array<IEntity>;
	ref array<int> 						combinations 				= new array<int>;
	
	vector 								m_WorldTransform[4];
	
	ref EntityPrefabData 					m_ItemSpawned;
	
	ref RandomGenerator 					m_RandomGenerator;
	
	CE_WorldValidationComponent 			m_WorldValidationComponent;
	
	CE_UsageTriggerArea					m_UsageArea;
	CE_TierTriggerArea					m_TierArea;
	
	
	
	
	//------------------------------------------------------------------------------------------------
	void CE_LootSpawning(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		m_RandomGenerator = new RandomGenerator();
	}

	//------------------------------------------------------------------------------------------------
	void ~CE_LootSpawning()
	{
		IEntity child = GetChildren();
		while (child)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(child);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		GetWorldTransform(m_WorldTransform);
		Init();
		DetermineLootPool();
		if(GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();

		if(m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.m_Processed)
				TryToSpawnLoot();
			else
				GetGame().GetCallqueue().CallLater(TryToSpawnLoot, 10000, false);
		}
	}

	void Init()
    {
		if (!m_sConfigs)
			return;		
			
		foreach(ResourceName m_ResourceName : m_sConfigs)
		{
			if(!m_ResourceName)
				continue;
			Resource m_Resource = BaseContainerTools.LoadContainer(m_ResourceName);
			BaseContainer m_Container = m_Resource.GetResource().ToBaseContainer();
			
			CE_LootSpawningConfig m_Config = CE_LootSpawningConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(m_Container));
			if(!m_Config)
				continue;

			foreach(CE_ItemData item: m_Config.m_ItemData)
			{
				if(!item.m_sName || !item.m_sPrefab)
					continue;
				m_aItems.Insert(item);
			}
		}
	}
	
	void DetermineLootPool()
	{
		for(int i = 0; i < m_aItems.Count(); i++)
		{
			//Print(m_aItems[i].m_ItemCategory);
			
			ref array<int> itemCategories = new array<int>;
			
			itemCategories.Insert(m_aItems[i].m_ItemCategory);
		}
		
		BaseWorld world = GetGame().GetWorld();
		
		if (world)
		{
			vector mins, maxs;
			
			world.GetBoundBox(mins, maxs);
			world.QueryEntitiesByAABB(mins, maxs, QueryEntitiesCallback, FilterEntitiesCallback, queryFlags: EQueryEntitiesFlags.STATIC);

		}
	}
	
	protected bool QueryEntitiesCallback(IEntity e)
	{	
		if (!e.IsInherited(CE_LootSpawning))
			return true;
		return true;
	}
	
	protected bool FilterEntitiesCallback(IEntity e)
	{	
		if (!e.IsInherited(CE_LootSpawning))
			return true;
		return true;
	}
	
	/*
	EntityPrefabData GetItemSpawned()
	{		
		return m_ItemSpawned;
	}
	
	/*
	void DetermineLootPool()
	{		
		//m_aItems.Clear();
		
		if (GetItemSpawned() == null)
		{
			for(int i = 0; i < m_aItems.Count(); i++)
			{
				SCR_Enum.GetEnumValues(CE_ELootCategory, m_aCategories);
			
				int categoryCount = m_aCategories.Count();
				
				for(int category = 1; category <= categoryCount; category++)
				{
					SCR_Enum.GetEnumValues(CE_ELootUsage, m_aUsages);
			
					int usageCount = m_aUsages.Count();
					
					SCR_Enum.GetEnumValues(CE_ELootTier, m_aTiers);
		
					int tierCount = m_aTiers.Count();
					
					array<int> m_aUsagesSubset = GenerateNonEmptySubsets(usageCount);
					array<int> m_aTiersSubset = GenerateNonEmptySubsets(tierCount);
					
					foreach(int u: m_aUsages)
					{
						foreach(int t: m_aTiers)
						{
							combinations.Reserve(1 + m_aUsagesSubset.Count() + m_aTiersSubset.Count());
							
							combinations.InsertAt(category, 0);
							
							combinations.InsertAt(u, 1);
							
							combinations.InsertAt(t, 1 + usageCount);
						}
					}
				}
			}
		
		}
		else
			return;
	}
	*/

/*
	
	static array<int> GenerateNonEmptySubsets(int numElements) // ty chatgpt
	{
		array<int> subsets = new array<int>;
		
		int totalSubsets = Math.Pow(2, numElements);
		
       	for (int subsetMask = 1; subsetMask < totalSubsets; subsetMask++)
		{
			array<int> subset = new array<int>();
			for (int element = 0; element < numElements; element++)
			{
				if ((subsetMask & (1 << element)) != 0)
				{
					subset.Insert(element + 1);
				}
			}
			subsets.Copy(subset);
		}
		return subsets;
	}
	
	
	protected void TryToSpawnLoot()
	{
		
		if(GetChildren())
			return;
		
		if(m_WorldValidationComponent.m_Processed)
		{
			Spawn();
			//Print(GetGame().GetPlayerManager());
		}
		else
		{
			GetGame().GetCallqueue().CallLater(TryToSpawnLoot, 10000, false);
		}
			
	}
	
	void Spawn()
	{
		// meow
	}
	
#ifdef WORKBENCH
	
	protected int m_iColor = Color.WHITE;
	protected string m_sText = string.Empty;
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetColorAndText()
	{
		if(m_Category == -1)
		{
			m_sText = "INVALID";
			m_iColor = Color.RED;
		}
		else
		{
			m_sText = SCR_Enum.GetEnumName(CE_ELootCategory, m_Category);
			m_iColor = Color.CYAN;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		SetColorAndText();

		vector mat[4];
		GetWorldTransform(mat);

		// Draw point and arrow
		vector position = mat[3];
		Shape pointShape = Shape.CreateSphere(m_iColor, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position, 0.2);
		Shape arrowShape = Shape.CreateArrow(position, position + mat[2], 0.2, m_iColor, ShapeFlags.ONCE);

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api)
		{
			IEntity selectEntity = api.SourceToEntity(api.GetSelectedEntity());
			if (!CE_LootSpawning.Cast(selectEntity))
				return;
		}

		if (!m_sText.IsEmpty())
		{
			DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
				GetWorld(),
				m_sText,
				DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
				position[0],
				position[1] + 0.4,
				position[2],
				20.0,
				m_iColor);
		}
	}
#endif
}
