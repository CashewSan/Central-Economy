[EntityEditorProps(category: "CentralEconomy/Entities", description: "")]
class CE_UsageTriggerAreaClass : ScriptedGameTriggerEntityClass
{
}

class CE_UsageTriggerArea : ScriptedGameTriggerEntity
{
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Which usage is this item spawn?", enums: ParamEnumArray.FromEnum(CE_ELootUsage), category: "Spawn Data")]
	CE_ELootUsage m_Usage;
	
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
				if (spawningComp && !spawningComp.m_ItemUsage)
				{
					spawningComp.SetSpawnerUsage(m_Usage);
				}
				
				CE_SearchableContainerComponent containerComp = CE_SearchableContainerComponent.Cast(entity.FindComponent(CE_SearchableContainerComponent));
				if (containerComp && !containerComp.m_ContainerUsage)
				{
					containerComp.SetContainerUsage(m_Usage);
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
	array<IEntity> GetUsageSpawnAreas()
	{
		return m_SpawnLocationsInside;
	}
}