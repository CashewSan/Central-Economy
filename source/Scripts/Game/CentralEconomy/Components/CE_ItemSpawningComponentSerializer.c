class CE_ItemSpawningComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return CE_ItemSpawningComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const CE_ItemSpawningComponent spawningComp = CE_ItemSpawningComponent.Cast(component);

		context.WriteValue("version", 1);
		context.WriteValue("readyForItem", spawningComp.IsReadyForItem());
		context.WriteValue("itemSpawned", spawningComp.GetItemSpawned());
		//context.WriteValue("entitySpawned", spawningComp.GetEntitySpawned());
		context.WriteValue("currentResetTime", spawningComp.GetCurrentSpawnerResetTime());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		CE_ItemSpawningComponent spawningComp = CE_ItemSpawningComponent.Cast(component);

		int version;
		context.Read(version);

		bool readyForItem;
		CE_Item itemSpawned;
		//IEntity entitySpawned;
		int currentResetTime;
		
		if (context.Read(readyForItem))
			spawningComp.SetReadyForItem(readyForItem);
		
		if (context.Read(itemSpawned))
			spawningComp.SetItemSpawned(itemSpawned);
		
		/*
		if (context.Read(entitySpawned))
			spawningComp.SetEntitySpawned(entitySpawned);
		*/
		
		if (context.Read(currentResetTime))
			spawningComp.SetCurrentSpawnerResetTime(currentResetTime);

		return true;
	}
}
