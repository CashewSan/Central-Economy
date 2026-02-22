modded class SCR_UniversalInventoryStorageComponent
{
	//------------------------------------------------------------------------------------------------
	override bool CanStoreItem(IEntity item, int slotID)
	{
		CE_SearchableContainerComponent searchableContainer = CE_SearchableContainerComponent.Cast(GetOwner().FindComponent(CE_SearchableContainerComponent));
		if (searchableContainer)
		{
			CE_ItemSpawnableComponent spawnableItem = CE_ItemSpawnableComponent.Cast(item.FindComponent(CE_ItemSpawnableComponent));
			if (spawnableItem)
			{
				if (!spawnableItem.WasDepositedByAction())
					return false;
			}
		}
		
		return super.CanStoreItem(item, slotID);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ShouldHideInVicinity()
	{
		CE_SearchableContainerComponent searchableContainer = CE_SearchableContainerComponent.Cast(GetOwner().FindComponent(CE_SearchableContainerComponent));
		if (searchableContainer)
		{
			//bool searched = searchableContainer.HasBeenSearched();
			return !searchableContainer.HasBeenSearched(); // if container has been searched, don't hide
		}
		
		return super.ShouldHideInVicinity();
	}
}