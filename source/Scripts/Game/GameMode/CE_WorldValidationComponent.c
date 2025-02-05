[ComponentEditorProps(category: "CentralEconomy/Components", description: "")]
class CE_WorldValidationComponentClass: SCR_BaseGameModeComponentClass
{
};

class CE_WorldValidationComponent: SCR_BaseGameModeComponent
{
	protected bool m_Processed = false;
	
	//protected const string m_sDbName = "CentralEconomyDatabase";
	
	protected const string DB_DIR = "$profile:/.CentralEconomy";
	protected const string DB_NAME = "CE_ItemData.json";
	
	ref CE_ItemDataJson m_ItemDataJson;
	
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		SetWorldProcessed(true);
		
		string m_sDb = string.Format("%1/%2", DB_DIR, DB_NAME);
		
		if (!FileIO.FileExists(m_sDb))
		{
			CreateItemData();
		}
		else
		{
			m_ItemDataJson = new CE_ItemDataJson;
			
			SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
			loadContext.LoadFromFile(m_sDb);
			
			loadContext.ReadValue("ItemData", m_ItemDataJson.m_ItemData);
		}
	}
	
	static CE_WorldValidationComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return CE_WorldValidationComponent.Cast(GetGame().GetGameMode().FindComponent(CE_WorldValidationComponent));
		else
			return null;
	}
	
	bool GetWorldProcessed()
	{
		return m_Processed;
	}
	
	void SetWorldProcessed(bool meow)
	{
		m_Processed = meow;
	}
	
	CE_ItemDataJson GetItemData()
	{
		return m_ItemDataJson;
	}
	
	protected void CreateItemData()
	{
		if (!FileIO.FileExists(DB_DIR))
		{
			FileIO.MakeDirectory(DB_DIR);
		}
		
		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext();
		PrettyJsonSaveContainer jsonContainer = new PrettyJsonSaveContainer();
		m_ItemDataJson = new CE_ItemDataJson();
		CE_ItemDataNEW starterItemData = StarterItemData();
		
		m_ItemDataJson.m_ItemData.Insert(starterItemData);
		
		ctx.SetContainer(jsonContainer);
		
		ctx.WriteValue("", m_ItemDataJson);
		
		string m_sDb = string.Format("%1/%2", DB_DIR, DB_NAME);
		
		jsonContainer.SaveToFile(m_sDb);
	}
	
	protected CE_ItemDataNEW StarterItemData()
	{
		CE_ItemDataNEW itemData = new CE_ItemDataNEW;
		
		itemData.m_sName = "CombatBoots_Soviet_01";
		itemData.m_sPrefab = "{C7923961D7235D70}Prefabs/Characters/Footwear/CombatBoots_Soviet_01.et";
		itemData.m_vItemRotation = "0 0 0";
		itemData.m_iNominal = 2;
		itemData.m_iMinimum = 1;
		itemData.m_iLifetime = 14400;
		itemData.m_iRestock = 3600;
		itemData.m_iQuantityMaximum = -1;
		itemData.m_iQuantityMinimum = -1;
		itemData.m_sItemCategory = SCR_Enum.GetEnumName(CE_ELootCategory, CE_ELootCategory.CLOTHING);
		itemData.m_sItemUsages = {SCR_Enum.GetEnumName(CE_ELootUsage, CE_ELootUsage.MILITARY), SCR_Enum.GetEnumName(CE_ELootUsage, CE_ELootUsage.POLICE)};
		itemData.m_sItemTiers = {SCR_Enum.GetEnumName(CE_ELootTier, CE_ELootTier.TIER2), SCR_Enum.GetEnumName(CE_ELootTier, CE_ELootTier.TIER3)};
		
		return itemData;
	}
};