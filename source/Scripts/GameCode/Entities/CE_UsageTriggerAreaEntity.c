[EntityEditorProps(category: "CentralEconomy/Entities", description: "")]
class CE_UsageTriggerAreaClass : ScriptedGameTriggerEntityClass
{
}

class CE_UsageTriggerArea : ScriptedGameTriggerEntity
{
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Which usage is this item spawn?", enums: ParamEnumArray.FromEnum(CE_ELootUsage), category: "Spawn Data")]
	CE_ELootUsage m_Usage;
	
	ref array<IEntity> m_SpawnLocationsInside = {};
	
	override void EOnInit(IEntity owner)
	{
		GatherSpawnLocations();
	}
	
	void GatherSpawnLocations()
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
				CE_ItemSpawningComponent lootComp = CE_ItemSpawningComponent.Cast(entity.FindComponent(CE_ItemSpawningComponent));
				
				lootComp.m_Usage = m_Usage;
			}
		}
	}  
	
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		if(!ent.FindComponent(CE_ItemSpawningComponent))
		{	
			return false;
		}
		return true;
	};
	
	array<IEntity> GetUsageSpawnAreas()
	{
		return m_SpawnLocationsInside;
	}
}