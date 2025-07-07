[BaseContainerProps(), BaseContainerCustomTitleField("m_sName")]
class CE_ItemData
{
	[Attribute("", UIWidgets.Auto, desc: "Name of Item **MUST BE UNIQUE**")]
	string m_sName;
	
    [Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Prefab to be spawned", "et")]
    ResourceName m_sPrefab;
	
	[Attribute("0 0 0", UIWidgets.EditBox, desc: "Euler angles of the item upon spawning (E.G. Weapons are typically x:0 y:0 z:90)")]
	vector m_vItemRotation;
	
	[Attribute("2", UIWidgets.EditBox, params: "1 10000", desc: "Ideal amount of spawns of this item around the map")]
	int m_iNominal;
	
	[Attribute("1", UIWidgets.EditBox, params: "1 10000", desc: "Minimum amount of spawns of this item around the map **SHOULD BE THE SAME OR LESS THAN THE NOMINAL VALUE**")]
	int m_iMinimum;
	
	[Attribute("14400", UIWidgets.EditBox, params: "0 3888000", desc: "Length of time (in seconds) item can stay on at a spawner before despawning")]
	int m_iLifetime;
	
	[Attribute("3600", UIWidgets.EditBox, params: "0 3888000", desc: "Length of time (in seconds) for item single instance to be added back to item pool after removal from spawner")]
	int m_iRestock;
	
	[Attribute("-1", UIWidgets.EditBox, params: "-1 100", desc: "Maximum quantity of item (only applies to weapon magazines/boxes, jerrycans fuel values, and vehicles fuel values)")]
	int m_iQuantityMaximum;
	
	[Attribute("-1", UIWidgets.EditBox, params: "-1 100", desc: "Minimum quantity of item (only applies to weapon magazines/boxes, jerrycans fuel values, and vehicles fuel values) **SHOULD BE THE SAME OR LESS THAN THE QUANTITY MAXIMUM VALUE**")]
	int m_iQuantityMinimum;
	
	[Attribute(ResourceName.Empty, UIWidgets.ComboBox, desc: "Which category does this item fall under?", enums: ParamEnumArray.FromEnum(CE_ELootCategory))]
	CE_ELootCategory m_ItemCategory;
	
	[Attribute("", UIWidgets.Flags, desc: "Which spawn location(s) will this item spawn in?", enums: ParamEnumArray.FromEnum(CE_ELootUsage))]
	CE_ELootUsage m_ItemUsages;
	
	[Attribute("", UIWidgets.Flags, desc: "Which tier(s) will this item spawn in?", enums: ParamEnumArray.FromEnum(CE_ELootTier))]
	CE_ELootTier m_ItemTiers;
}

[BaseContainerProps(configRoot: true)]
class CE_ItemDataConfig
{
	[Attribute(UIWidgets.Object, desc: "Item Data")]
    ref array<ref CE_ItemData> m_ItemData;
	
	void ~CE_ItemDataConfig()
	{
	}
};







