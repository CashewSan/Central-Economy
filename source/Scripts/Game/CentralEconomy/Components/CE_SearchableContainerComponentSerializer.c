class CE_SearchableContainerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return CE_SearchableContainerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const CE_SearchableContainerComponent containerComp = CE_SearchableContainerComponent.Cast(component);

		context.WriteValue("version", 1);
		context.WriteValue("beenSearched", containerComp.HasBeenSearched());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		CE_SearchableContainerComponent containerComp = CE_SearchableContainerComponent.Cast(component);

		int version;
		context.Read(version);

		bool beenSearched;
		
		if (context.Read(beenSearched))
		{
			if (beenSearched == true)
			{
				SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_InventoryStorageManagerComponent));
				if (storageManager)
				{
					array<IEntity> outItems = {};
					storageManager.GetItems(outItems);
					
					foreach (IEntity item : outItems)
					{
						storageManager.TryDeleteItem(item);
					}
				}
				
				containerComp.SetHasBeenSearched(false);
			}
		}

		return true;
	}
}
