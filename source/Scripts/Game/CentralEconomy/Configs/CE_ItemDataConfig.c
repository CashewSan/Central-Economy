[BaseContainerProps(), BaseContainerCustomTitleField("m_sName")]
class CE_ItemData
{
	[Attribute("", UIWidgets.Auto, desc: "Name of Item **MUST BE UNIQUE**")]
	protected string m_sName;
	
    [Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Prefab to be spawned", "et")]
    protected ResourceName m_sPrefab;
	
	[Attribute("0 0 0", UIWidgets.EditBox, desc: "Euler angles of the item upon spawning (E.G. Weapons are typically x:0 y:0 z:90)")]
	protected vector m_vItemRotation;
	
	[Attribute("40", UIWidgets.EditBox, params: "1 10000", desc: "Ideal amount of spawns of this item around the map")]
	protected int m_iNominal;
	
	[Attribute("30", UIWidgets.EditBox, params: "1 10000", desc: "Minimum amount of spawns of this item around the map **SHOULD BE THE SAME OR LESS THAN THE NOMINAL VALUE**")]
	protected int m_iMinimum;
	
	[Attribute("14400", UIWidgets.EditBox, params: "0 3888000", desc: "Length of time (in seconds) item can stay on at a spawner before despawning")]
	protected int m_iLifetime;
	
	[Attribute("3600", UIWidgets.EditBox, params: "0 3888000", desc: "Length of time (in seconds) for item single instance to be added back to item pool after removal from spawner")]
	protected int m_iRestock;
	
	[Attribute("-1", UIWidgets.EditBox, params: "-1 100", desc: "Maximum quantity of item (only applies to weapon magazines/boxes, jerrycans fuel values, and vehicles fuel values)")]
	protected int m_iQuantityMaximum;
	
	[Attribute("-1", UIWidgets.EditBox, params: "-1 100", desc: "Minimum quantity of item (only applies to weapon magazines/boxes, jerrycans fuel values, and vehicles fuel values) **SHOULD BE THE SAME OR LESS THAN THE QUANTITY MAXIMUM VALUE**")]
	protected int m_iQuantityMinimum;
	
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Which category does this item fall under?", enums: ParamEnumArray.FromEnum(CE_ELootCategory))]
	protected CE_ELootCategory m_ItemCategory;
	
	[Attribute("", UIWidgets.Flags, desc: "Which spawn location(s) will this item spawn in?", enums: ParamEnumArray.FromEnum(CE_ELootUsage))]
	protected CE_ELootUsage m_ItemUsages;
	
	[Attribute("", UIWidgets.Flags, desc: "Which tier(s) will this item spawn in?", enums: ParamEnumArray.FromEnum(CE_ELootTier))]
	protected CE_ELootTier m_ItemTiers;
	
	//------------------------------------------------------------------------------------------------
	//! Returns the name of the CE_ItemData
	string GetName()
	{
		return m_sName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the prefab of the CE_ItemData
	ResourceName GetPrefab()
	{
		return m_sPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the rotation of the CE_ItemData
	vector GetRotation()
	{
		return m_vItemRotation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the nominal of the CE_ItemData
	int GetNominal()
	{
		return m_iNominal;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the minimum of the CE_ItemData
	int GetMinimum()
	{
		return m_iMinimum;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the lifetime of the CE_ItemData
	int GetLifetime()
	{
		return m_iLifetime;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the restock of the CE_ItemData
	int GetRestock()
	{
		return m_iRestock;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the quantity maximum of the CE_ItemData
	int GetQuantityMaximum()
	{
		return m_iQuantityMaximum;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the quantity minimum of the CE_ItemData
	int GetQuantityMinimum()
	{
		return m_iQuantityMinimum;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the category of the CE_ItemData
	CE_ELootCategory GetCategory()
	{
		return m_ItemCategory;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the usage(s) of the CE_ItemData
	CE_ELootUsage GetUsages()
	{
		return m_ItemUsages;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the tier(s) of the CE_ItemData
	CE_ELootTier GetTiers()
	{
		return m_ItemTiers;
	}
}

[BaseContainerProps(configRoot: true)]
class CE_ItemDataConfig
{
	[Attribute(UIWidgets.Object, desc: "Item Data")]
    protected ref array<ref CE_ItemData> m_ItemData;
	
	void ~CE_ItemDataConfig()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ItemData array of the CE_ItemDataConfig
	array<ref CE_ItemData> GetItemDataArray()
	{
		return m_ItemData;
	}
};







