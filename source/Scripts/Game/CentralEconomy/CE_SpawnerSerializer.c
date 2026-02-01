/*
class CE_SpawnerData : PersistentState
{
}

class CE_SpawnerSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return CE_Spawner;
	}
	
	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		//Print("CE TESTMEOW: " + instance);
		
		const CE_Spawner spawnerInstance = CE_Spawner.Cast(instance);
		
		context.WriteValue("version", 1);
		context.WriteValue("itemSpawned", spawnerInstance.GetItemSpawned());
		context.WriteValue("readyForItem", spawnerInstance.IsReadyForItem());
		return ESerializeResult.OK;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		//Print("CE TESTMEOW: " + instance);
		
		CE_Spawner spawnerInstance = CE_Spawner.Cast(instance);
		
		int version;
		context.Read(version);

		CE_Item itemSpawned;
		bool readyForItem;
		
		if (context.Read(itemSpawned))
			spawnerInstance.SetItemSpawned(itemSpawned);
		
		if (context.Read(readyForItem))
			spawnerInstance.SetReadyForItem(readyForItem);

		return true;
	}
}