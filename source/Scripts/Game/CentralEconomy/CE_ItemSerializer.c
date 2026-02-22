/*
class CE_ItemSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return CE_Item;
	}
	
	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		const CE_Item itemInstance = CE_Item.Cast(instance);
		
		context.WriteValue("version", 1);
		context.WriteValue("itemData", itemInstance.GetItemData());
		context.WriteValue("availableCount", itemInstance.GetAvailableCount());
		return ESerializeResult.OK;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		CE_Item itemInstance = CE_Item.Cast(instance);
		
		int version;
		context.Read(version);

		int availableCount;
		CE_ItemData itemData;
		
		if (context.Read(availableCount))
			itemInstance.SetAvailableCount(availableCount);
		
		if (context.Read(itemData))
			itemInstance.SetItemData(itemData);

		return true;
	}
}