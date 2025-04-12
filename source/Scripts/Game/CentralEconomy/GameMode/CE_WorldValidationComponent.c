[ComponentEditorProps(category: "CentralEconomy/Components", description: "World validation component that loads or creates the CE_ItemDataConfig file in server profile folder.")]
class CE_WorldValidationComponentClass: SCR_BaseGameModeComponentClass
{
};

class CE_WorldValidationComponent: SCR_BaseGameModeComponent
{
	protected bool 											m_Processed 				= false;							// has world been processed?
	
	protected ref CE_ItemDataConfig 							m_ItemDataConfig;										// universal config from server profile folder
	
	protected const string 									DB_DIR 					= "$profile:/CentralEconomy";		// directory name in the server profile folder
	protected const string 									DB_NAME_CONF 			= "CE_ItemData.conf";				// config file name in the server profile folder
	
	//------------------------------------------------------------------------------------------------
	//!
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		SetWorldProcessed(true);
		
		if (Replication.IsServer()) // only create the config or load config if you're the server
		{
			string m_sDb = string.Format("%1/%2", DB_DIR, DB_NAME_CONF);
		
			if (!FileIO.FileExists(m_sDb))
			{
				CreateConfig();
			}
			else
			{
				Resource holder = BaseContainerTools.LoadContainer(m_sDb);
				
				m_ItemDataConfig = CE_ItemDataConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
				
				if (!m_ItemDataConfig || !m_ItemDataConfig.m_ItemData)
				{
					Print("[CentralEconomy] CE_ItemData.conf can not be found! This is typically located in your server's profile folder, please resolve!", LogLevel.ERROR);
					return;
				}
				
				if (m_ItemDataConfig.m_ItemData.IsEmpty())
				{
					Print("[CentralEconomy] CE_ItemData.conf is empty! This is located in your server's profile folder, please resolve!", LogLevel.ERROR);
					return;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static CE_WorldValidationComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return CE_WorldValidationComponent.Cast(GetGame().GetGameMode().FindComponent(CE_WorldValidationComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetWorldProcessed()
	{
		return m_Processed;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWorldProcessed(bool meow)
	{
		m_Processed = meow;
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ItemDataConfig GetItemDataConfig()
	{
		return m_ItemDataConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateConfig()
	{
		if (Replication.IsServer()) // only create the config or load config if you're the server
		{
			if (!FileIO.FileExists(DB_DIR))
			{
				FileIO.MakeDirectory(DB_DIR);
			}
			
			ResourceName m_sDb = string.Format("%1/%2", DB_DIR, DB_NAME_CONF);
			
			CE_ItemDataConfig obj = new CE_ItemDataConfig();
			
			Resource holder = BaseContainerTools.CreateContainerFromInstance(obj);
			
			BaseContainerTools.SaveContainer(holder.GetResource().ToBaseContainer(), m_sDb);
			
			Print("[CentralEconomy] CE_ItemData.conf created! Please add items to config and then restart server!");
		}
	}
	
	// Old stuff that I'm keeping for reference
	/*
	//------------------------------------------------------------------------------------------------
	CE_ItemDataJson GetItemData()
	{
		return m_ItemDataJson;
	}
	
	//------------------------------------------------------------------------------------------------
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
	
	//------------------------------------------------------------------------------------------------
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
	*/
};