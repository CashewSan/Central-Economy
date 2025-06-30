[EntityEditorProps(category: "CentralEconomy/Entities", description: "")]
class CE_TierTriggerAreaClass : ScriptedGameTriggerEntityClass
{
}

class CE_TierTriggerArea : ScriptedGameTriggerEntity
{
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Which tier is this item spawn?", enums: ParamEnumArray.FromEnum(CE_ELootTier), category: "Spawn Data")]
	CE_ELootTier m_Tier;
	
	ref array<IEntity> m_SpawnLocationsInside = {};
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		GatherSpawnLocations();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GatherSpawnLocations()
	{	
		QueryEntitiesInside();
		GetEntitiesInside(m_SpawnLocationsInside);
		
		if (m_SpawnLocationsInside.Count() == 0)
		{
			GetGame().GetCallqueue().CallLater(GatherSpawnLocations, 100, false);
		}
		else
		{
			foreach (IEntity entity : m_SpawnLocationsInside)
			{
				CE_ItemSpawningComponent spawningComp = CE_ItemSpawningComponent.Cast(entity.FindComponent(CE_ItemSpawningComponent));
				if (spawningComp)
				{
					spawningComp.SetSpawnerTier(m_Tier);
				}
				
				CE_SearchableContainerComponent containerComp = CE_SearchableContainerComponent.Cast(entity.FindComponent(CE_SearchableContainerComponent));
				if (containerComp)
				{
					containerComp.SetContainerTier(m_Tier);
				}
			}
		}
	}  
	
	//------------------------------------------------------------------------------------------------
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if (ent.FindComponent(CE_ItemSpawningComponent) || ent.FindComponent(CE_SearchableContainerComponent))
		{	
			return true;
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	array<IEntity> GetTierSpawnAreas()
	{
		return m_SpawnLocationsInside;
	}
}