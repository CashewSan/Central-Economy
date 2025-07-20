[EntityEditorProps(category: "CentralEconomy/Entities", description: "")]
class CE_UsageTriggerAreaClass : SCR_BaseTriggerEntityClass
{
}

class CE_UsageTriggerArea : SCR_BaseTriggerEntity
{	
	[Attribute(ResourceName.Empty, uiwidget: UIWidgets.ComboBox, desc: "Which usage is this area?", enums: ParamEnumArray.FromEnum(CE_ELootUsage), category: "Spawn Data")]
	protected CE_ELootUsage m_Usage;
	
	ref array<IEntity> m_aSpawnLocationsInside = {};
	
	protected CE_ItemSpawningSystem m_SpawningSystem;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		super.EOnInit(owner);
		
		if (GetGame().InPlayMode())
		{
			GetOnQueryFinished().Insert(GatherSpawnLocations);
			
			QueryEntitiesInside();
			
			ConnectToItemSpawningSystem();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void GatherSpawnLocations()
	{
		GetEntitiesInside(m_aSpawnLocationsInside);
		
		if (m_aSpawnLocationsInside.IsEmpty())
		{
			DisconnectFromItemSpawningSystem();
		}
		else
			SetUsage();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void SetUsage()
	{
		int spawnerCount = 0;
		int containerCount = 0;
		int spawnerSetCount = 0;
		int containerSetCount = 0;
		
		foreach (IEntity entity : m_aSpawnLocationsInside)
		{
			CE_ItemSpawningComponent spawningComp = CE_ItemSpawningComponent.Cast(entity.FindComponent(CE_ItemSpawningComponent));
			if (spawningComp && !spawningComp.HasUsage() && m_Usage)
			{
				spawnerCount++;
				
				spawningComp.SetSpawnerUsage(m_Usage);
				
				if (spawningComp.GetSpawnerUsage())
				{
					spawnerSetCount++;
				}
			}
			
			CE_SearchableContainerComponent containerComp = CE_SearchableContainerComponent.Cast(entity.FindComponent(CE_SearchableContainerComponent));
			if (containerComp && !containerComp.HasUsage() && m_Usage)
			{
				containerCount++;
				
				containerComp.SetContainerUsage(m_Usage);
				
				if (containerComp.GetContainerUsage())
				{
					containerSetCount++;
				}
			}
		}
		
		if (spawnerSetCount >= spawnerCount && containerSetCount >= containerCount)
		{
			World world = GetWorld();
			if (!world)
				return;
			
			m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
			if (!m_SpawningSystem)
				return;
			
			m_SpawningSystem.SetUsageAreasQueried(m_SpawningSystem.GetUsageAreasQueried() + 1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		return ent.FindComponent(CE_ItemSpawningComponent) || ent.FindComponent(CE_SearchableContainerComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Connects to item spawning system and registers this area
	protected void ConnectToItemSpawningSystem()
	{
		World world = GetWorld();
		if (!world)
			return;
		
		m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
		if (!m_SpawningSystem)
			return;
		
		m_SpawningSystem.RegisterUsageArea(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this area
	protected void DisconnectFromItemSpawningSystem()
	{
		World world = GetWorld();
		if (!world)
			return;
		
		m_SpawningSystem = CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
		if (!m_SpawningSystem)
			return;

		m_SpawningSystem.UnregisterUsageArea(this);
	}
}