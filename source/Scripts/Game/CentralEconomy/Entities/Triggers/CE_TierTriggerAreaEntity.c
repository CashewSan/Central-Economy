[EntityEditorProps(category: "CentralEconomy/Entities", description: "")]
class CE_TierTriggerAreaClass : SCR_BaseTriggerEntityClass
{
}

class CE_TierTriggerArea : SCR_BaseTriggerEntity
{
	[Attribute(ResourceName.Empty, uiwidget: UIWidgets.ComboBox, desc: "Which tier is this area?", enums: ParamEnumArray.FromEnum(CE_ELootTier), category: "Spawn Data")]
	protected CE_ELootTier m_Tier;
	
	protected ref array<IEntity> m_aSpawnLocationsInside = {};
	
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
			SetTier();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void SetTier()
	{
		int spawnerCount = 0;
		int spawnerSetCount = 0;
		
		foreach (IEntity entity : m_aSpawnLocationsInside)
		{
			CE_ItemSpawningComponent spawningComp = CE_ItemSpawningComponent.Cast(entity.FindComponent(CE_ItemSpawningComponent));
			if (m_Tier && spawningComp)
			{
				spawnerCount++;
				
				spawningComp.SetSpawnerTier(m_Tier);
				
				if (spawningComp.GetSpawnerTier())
				{
					spawnerSetCount++;
				}
			}
			
			CE_SearchableContainerComponent containerComp = CE_SearchableContainerComponent.Cast(entity.FindComponent(CE_SearchableContainerComponent));
			if (m_Tier && containerComp)
			{
				containerComp.SetContainerTier(m_Tier);
			}
		}
		
		if (spawnerSetCount >= spawnerCount)
		{
			m_SpawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(this);
			if (!m_SpawningSystem)
				return;
			
			m_SpawningSystem.SetTierAreasQueried(m_SpawningSystem.GetTierAreasQueried() + 1);
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
		m_SpawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(this);
		if (!m_SpawningSystem)
			return;
		
		m_SpawningSystem.RegisterTierArea(this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disonnects from item spawning system and unregisters this area
	protected void DisconnectFromItemSpawningSystem()
	{
		m_SpawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(this);
		if (!m_SpawningSystem)
			return;

		m_SpawningSystem.UnregisterTierArea(this);
	}
}