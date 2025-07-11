[ComponentEditorProps(category: "CentralEconomy/Components", description: "World validation component that loads or creates the CE_ItemDataConfig file in server profile folder.")]
class CE_WorldValidationComponentClass: SCR_BaseGameModeComponentClass
{
};

class CE_WorldValidationComponent: SCR_BaseGameModeComponent
{
	[Attribute("5", UIWidgets.EditBox, desc: "Frequency (in seconds) that an item will spawn, LOWER TAKES MORE PERFORMANCE (E.G. If set to 5, an item will attempt to spawn every 5 seconds)", params: "0 inf 1", category: "Item Spawning System")] // default set to 1 second
	float m_fItemSpawningFrequency;
	
	[Attribute("0.5", UIWidgets.EditBox, desc: "Ratio of items the system will aim to spawn compared to spawners (If set to 0.5, items will populate half of the spawners in the world. If set to 1, items will populate all spawners)", params: "0 1 0.1", category: "Item Spawning System")] // default set to 1 second
	float m_fItemSpawningRatio;
	
	protected bool 											m_bProcessed 			= false;							// has world been processed?
	
	protected ref CE_ItemDataConfig 							m_ItemDataConfig;										// universal config from server profile folder
	
	protected const string 									DB_DIR 					= "$profile:/CentralEconomy";		// directory name in the server profile folder
	protected const string 									DB_NAME_CONF 			= "CE_ItemData.conf";				// config file name in the server profile folder
	
	//------------------------------------------------------------------------------------------------
	//! Called on post process of the world, sets to find universal config or create it if it doesn't exist
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
					Print("[CentralEconomy::CE_WorldValidationComponent] CE_ItemData.conf can not be found! This is typically located in your server's profile folder, please resolve!", LogLevel.ERROR);
					return;
				}
				
				if (m_ItemDataConfig.m_ItemData.IsEmpty())
				{
					Print("[CentralEconomy::CE_WorldValidationComponent] CE_ItemData.conf is empty! This is located in your server's profile folder, please resolve!", LogLevel.ERROR);
					return;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates item data config in server profile folder
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
			
			Print("[CentralEconomy] CE_ItemData.conf created in your server profile folder! Please add items to config and then restart server!");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// GETTERS/SETTERS
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Gets the instance of the CE_WorldValidationComponent
	static CE_WorldValidationComponent GetInstance()
	{
		if (GetGame().GetGameMode())
			return CE_WorldValidationComponent.Cast(GetGame().GetGameMode().FindComponent(CE_WorldValidationComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Has world been processed?
	bool HasWorldProcessed()
	{
		return m_bProcessed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the world has been processed
	void SetWorldProcessed(bool processed)
	{
		m_bProcessed = processed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets item data config
	CE_ItemDataConfig GetItemDataConfig()
	{
		return m_ItemDataConfig;
	}
};