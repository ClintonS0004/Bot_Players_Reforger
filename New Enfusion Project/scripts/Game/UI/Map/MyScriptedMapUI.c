modded enum ChimeraMenuPreset {
    SCR_MapsBotsMenuUI
}

class myNewLayoutClass: SCR_SuperMenuBase
{
  //your GUI code ...    
	
	// Add GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.myTestGui_ID); 
	// somewhere else
}

class SPK_myMenuUI: SCR_SuperMenuBase
{    
	// please copy SCR_PlayerListMenu into here
	// this class is registered in chimeraMenus.conf
		
	protected ref array<ref SCR_PlayerListEntry> m_aEntries = new array<ref SCR_PlayerListEntry>();
	protected ref map<int, SCR_ScoreInfo> m_aAllPlayersInfo = new map<int, SCR_ScoreInfo>();
	protected ResourceName m_sScoreboardRow = "{65369923121A38E7}UI/layouts/Menus/PlayerList/PlayerListEntry.layout";
	protected Widget m_wTable;
	protected ref array<Faction> m_aFactions = {null};
	
	protected static const ResourceName FACTION_COUNTER_LAYOUT = "{5AD2CE85825EDA11}UI/layouts/Menus/PlayerList/FactionPlayerCounter.layout";
	
	protected string m_sGameMasterIndicatorName = "GameMasterIndicator";
	
	protected const int DEFAULT_SORT_INDEX = 1;
	
	protected SCR_InputButtonComponent m_Friend;
	protected SCR_InputButtonComponent m_Block;
	protected SCR_InputButtonComponent m_Mute;
	protected SCR_InputButtonComponent m_Vote;
	protected SCR_InputButtonComponent m_Invite;
 	protected SCR_InputButtonComponent m_Unblock;
	protected SCR_InputButtonComponent m_ViewProfile;
	
	protected SCR_VotingManagerComponent m_VotingManager;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupController;
	
	protected PlayerController m_PlayerController;
	protected SCR_BaseScoringSystemComponent m_ScoringSystem;
	SCR_SortHeaderComponent m_Header;
	protected SCR_PlayerListEntry m_SelectedEntry;
	protected SocialComponent m_SocialComponent;
	
	protected const string FILTER_NAME = "Name";
	protected const string FILTER_FREQ = "Freq";
	
	protected const string MUTE = "#AR-PlayerList_Mute";
	protected const string UNMUTE = "#AR-PlayerList_Unmute";
	protected const string ADD_FRIEND = "#AR-PlayerList_AddFriend";
	protected const string REMOVE_FRIEND = "#AR-PlayerList_RemoveFriend";
	
	protected static ref ScriptInvoker s_OnPlayerListMenu = new ScriptInvoker();
	
	protected ref Color m_PlayerNameSelfColor = new Color(0.898, 0.541, 0.184, 1);
	
	/*!
	Get event called when player list opens or closes.
	\return Script invoker
	*/
	static ScriptInvoker GetOnPlayerListMenu()
	{
		return s_OnPlayerListMenu;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitSorting()
	{
		if (!GetRootWidget())
			return;

		Widget w = GetRootWidget().FindAnyWidget("SortHeader");
		if (!w)
			return;

		m_Header = SCR_SortHeaderComponent.Cast(w.FindHandler(SCR_SortHeaderComponent));
		if (!m_Header)
			return;

		m_Header.m_OnChanged.Insert(OnHeaderChanged);

		if (m_ScoringSystem)
			return;

		// Hide K/D/S sorting headers if the re is no scoreboard
		ButtonWidget sortKills = ButtonWidget.Cast(w.FindAnyWidget("sortKills"));
		ButtonWidget sortDeaths = ButtonWidget.Cast(w.FindAnyWidget("sortDeaths"));
		ButtonWidget sortScore = ButtonWidget.Cast(w.FindAnyWidget("sortScore"));

		if (sortKills)
			sortKills.SetOpacity(0);
		if (sortDeaths)
			sortDeaths.SetOpacity(0);
		if (sortScore)
			sortScore.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHeaderChanged(SCR_SortHeaderComponent sortHeader)
	{
		string filterName = sortHeader.GetSortElementName();
		bool sortUp = sortHeader.GetSortOrderAscending();
		Sort(filterName, sortUp);
	}

	
	//------------------------------------------------------------------------------------------------
	void SortByName(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		array<string> names = {};
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_wName)
				names.Insert(entry.m_wName.GetText());
		}

		names.Sort();

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (!entry.m_wName)
				continue;

			string text = entry.m_wName.GetText();

			foreach (int i, string s : names)
			{
				if (s != text)
					continue;

				if (entry.m_wRow)
					entry.m_wRow.SetZOrder(i * direction);
				continue;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByFrequency(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			entry.m_wRow.SetZOrder(entry.m_iSortFrequency * direction);
		}
	}
	//------------------------------------------------------------------------------------------------
	protected void Sort(string filterName, bool sortUp)
	{
		if (filterName == FILTER_NAME)
			SortByName(sortUp);
		else if (filterName == FILTER_FREQ)
			SortByFrequency(sortUp);
	}

	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		//if (!m_ChatPanel)
		//	m_ChatPanel = SCR_ChatPanel.Cast(m_wRoot.FindAnyWidget("ChatPanel").FindHandler(SCR_ChatPanel));		
		
		m_PlayerController = GetGame().GetPlayerController();
		if (m_PlayerController)
			m_SocialComponent = SocialComponent.Cast(m_PlayerController.FindComponent(SocialComponent));
		
		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(m_PlayerController.FindComponent(SCR_HUDManagerComponent));
		hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() & ~EHudLayers.HIGH);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		m_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		m_PlayerController = GetGame().GetPlayerController();


		FactionManager fm = GetGame().GetFactionManager();
		if (fm)
		{
			fm.GetFactionsList(m_aFactions);
		}

		m_wTable = GetRootWidget().FindAnyWidget("Table");

		// Create navigation buttons
		Widget footer = GetRootWidget().FindAnyWidget("FooterLeft");
		Widget footerBack = GetRootWidget().FindAnyWidget("Footer");
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, footerBack);
		if (back)
		{
			back.m_OnActivated.Insert(OnBack);  // It's OnActivated not OnClicked
			
			//back.SetLabel("Make my Tab");
		}
		// Set button actionsS
		/*
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		SCR_ButtonBaseComponent add = SCR_ButtonBaseComponent.GetButtonBase("Mute", w);
		if (add)
		{
			add.SetEnabled(true);
			add.m_OnClicked.Insert(OnAddClick);
		} else
		{
			Print("Add button could not be made!", LogLevel.ERROR);
		}*/
		
		// Add my buttons
		m_Mute = SCR_InputButtonComponent.GetInputButtonComponent("Mute", footer);
		if (m_Mute)
		{
			m_Mute.SetEnabled(true);
			m_Mute.m_OnActivated.Insert(OnAddClick);
			m_Mute.SetLabel(" Add ");
		}
		
		m_Block = SCR_InputButtonComponent.GetInputButtonComponent("Block", footer);
		if (m_Block)
		{
			m_Block.SetEnabled(true);
			m_Block.m_OnActivated.Insert(OnRemoveClick);
			m_Block.SetLabel("Remove");
		}
		
		//SetupComboBoxDropdown(m_aFactions);

		
		// Create table
		if (!m_wTable || m_sScoreboardRow == string.Empty)
			return;

		//Get editor Delegate manager to check if has editor rights
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));

		// #### Replace this
		array<int> ids = {};
		
		array<ref CLINTON_Virtual_Player> players = CLINTON_RoboPlayerManager.GetPlayers();
		// GetGame().GetPlayerManager().GetPlayers(ids);
		
		for (int i = 0; i < players.Count(); i++) // starts from 1 to not have 0-based index miscalculation
		{
				CreateEntry(i, editorDelegateManager);
		}
		
		InitSorting();
		
		m_SuperMenuComponent.GetTabView().GetOnChanged().Insert(OnTabChanged);
		
		// Create new tabs
		SCR_Faction scrFaction;
		foreach (Faction faction : m_aFactions)
		{
			if (!faction)
				continue;

			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable())
				continue; //--- ToDo: Refresh dynamically when a new faction is added/removed
			
			string name = faction.GetFactionName();
			m_SuperMenuComponent.GetTabView().AddTab(ResourceName.Empty,name);
			
			AddFactionPlayerCounter(faction);
		}

		//handle groups tab
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			playerFaction = factionManager.GetLocalPlayerFaction();
		
		
		if (!groupsManager || !playerFaction)
			m_SuperMenuComponent.GetTabView().SetTabVisible(EPlayerListTab.GROUPS, false);
		//UpdateFrequencies();
		
		//---------------------------------------------------------------------------
		// Get Combo box widget and populate with the factions list
		// add invokers for choosing an option and initialise the memory
		// TODO: Add a Faction key as data
		// scr.AddItem( "Each Faction", false );
		XComboBoxWidget factionBox = XComboBoxWidget.Cast(GetRootWidget().FindAnyWidget("CLINTON_ComboBox"));
		factionBox.AddItem("Each Faction");
		foreach (Faction faction : m_aFactions)
		{
			if (!faction)
				continue;

			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable())
				continue; //--- ToDo: Refresh dynamically when a new faction is added/removed
			
			string name = faction.GetFactionName();
			factionBox.AddItem( name );  // returns int as index
		}
		
		Widget w = GetRootWidget().FindAnyWidget("CLINTON_ComboRoot");
		// Idk why the ComboBoxComponent is the first Handler
		// if this breaks find a way to get the specific Handler irespective of pos
		SCR_ComboBoxComponent scr = SCR_ComboBoxComponent.Cast(w.GetHandler(0));
		if(!scr) Print("Wrong Handler / Controller", LogLevel.ERROR);
		
		scr.AddItem( "Join Groups based on distance", false );
		scr.AddItem( "Fill Player groups first", false );
		scr.AddItem( "Cohesive Groups", true );
		
		scr.GetCurrentIndex();
		
		s_OnPlayerListMenu.Invoke(true);
	}
	
	// Parity check when doing replication
	
	// Consider 'updating' the player list by directly deleting or adding layouts
	// Avoids events like deleting one player in a list of one hundred
	//------------------------------------------------------------------------------------------------
	void CreateEntry(int id, SCR_PlayerDelegateEditorComponent editorDelegateManager, Faction faction = null)
	{
		//check for existing entry, return if it exists already
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_iID == id)
				return;
		}

		ImageWidget badgeTop, badgeMiddle, badgeBottom;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		if (!w)
			return;

		SCR_PlayerListEntry entry = new SCR_PlayerListEntry();
		entry.m_iID = id;
		entry.m_wRow = w;

		//--- 'rid the voting

		SCR_ButtonBaseComponent handler = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (handler)
		{
			handler.m_OnFocus.Insert(OnEntryFocused);
			handler.m_OnFocusLost.Insert(OnEntryFocusLost);
		}

		if (m_aAllPlayersInfo)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)
			{
				if (!info || playerId != id)
					continue;

				entry.m_Info = info;
				break;
			}
		}

		// Find faction
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!faction)
		{
			faction = factionManager.GetPlayerFaction(entry.m_iID);
			//string factionK = CLINTON_RoboPlayerManager.GetPlayer(entry.m_iID).GetFactionKey();
			//Faction faction = factionManager.GetFactionByKey(factionK);
		}
		entry.m_Faction = faction;

		Widget factionImage = w.FindAnyWidget("FactionImage");

		if (factionImage)
		{
			if (entry.m_Faction)
				factionImage.SetColor(entry.m_Faction.GetFactionColor());
			else
				factionImage.SetVisible(false);
		}

		entry.m_wName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));

		if (entry.m_wName)
		{
			CLINTON_RoboPlayerManager pm = CLINTON_RoboPlayerManager.getInstance();
			if(pm){
				//delayed_name_set(entry, id);
				entry.m_wName.SetText(pm.GetVirtualPlayerName(id));
			}
			if (entry.m_iID == m_PlayerController.GetPlayerId())
				entry.m_wName.SetColor(m_PlayerNameSelfColor);
		}


		entry.m_wFreq = TextWidget.Cast(w.FindAnyWidget("Freq"));
		// entry.m_wKills = TextWidget.Cast(w.FindAnyWidget("Kills"));
		// entry.m_wDeaths = TextWidget.Cast(w.FindAnyWidget("Deaths"));
		// entry.m_wScore = TextWidget.Cast(w.FindAnyWidget("Score"));
		if (entry.m_Info)
		{
			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(id);

				entry.m_wScore.SetText(score.ToString());
			}
		}
		else
		{

			if (entry.m_wKills)
				entry.m_wKills.SetText("");
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText("");
			if (entry.m_wScore)
				entry.m_wScore.SetText("");
			// Unfortunately the parent that must be hidden is two parents above the text widgets
			/*
			if (entry.m_wKills)
				entry.m_wKills.GetParent().GetParent().SetVisible(false);
			if (entry.m_wDeaths)
				entry.m_wDeaths.GetParent().GetParent().SetVisible(false);
			if (entry.m_wScore)
				entry.m_wScore.GetParent().GetParent().SetVisible(false);
			*/
		}
		ImageWidget background = ImageWidget.Cast(w.FindAnyWidget("Background")); /*
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup group = groupManager.GetPlayerGroup(m_PlayerController.GetPlayerId());

		if (group && group.HasRequesterID(id))
			background.SetColor(m_PlayerNameSelfColor);		// TODO: Idk I stole SCR_PlayerListMenu

		if (entry.m_wTaskIcon && GetTaskManager())
		{
			SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.FindTaskExecutorByID(entry.m_iID);
			if (taskExecutor.GetAssignedTask())
			{
				entry.m_wTaskIcon.SetColor(entry.m_Faction.GetFactionColor());
			}
			else
			{
				entry.m_wTaskIcon.SetOpacity(0);
			}
		}*/

		Faction playerFaction;		
		Faction entryPlayerFaction;
		if (!faction)
		{
			CLINTON_RoboPlayerManager pm = CLINTON_RoboPlayerManager.getInstance();
			playerFaction = factionManager.GetFactionByKey(pm.GetVirtualPlayerFactionKey(id));
			entry.m_Faction = playerFaction;
		}

		badgeTop = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeBottom"));
		Color factionColor;

		if (badgeTop && badgeMiddle && badgeBottom && entry.m_Faction)
		{
			factionColor = entry.m_Faction.GetFactionColor();
			badgeTop.SetColor(factionColor);
			badgeMiddle.SetColor(factionColor);
			badgeBottom.SetColor(factionColor);
		}
		
		m_aEntries.Insert(entry);
	}
	
	void delayed_name_set(SCR_PlayerListEntry entry, int bot_id)
	{
		CLINTON_RoboPlayerManager pm = CLINTON_RoboPlayerManager.getInstance();
		CLINTON_Virtual_Player bot = pm.GetPlayer(bot_id);
	
		if(!bot)
		{
			GetGame().GetCallqueue().CallLater(delayed_name_set, 2048, true, entry, bot_id);
			return;
		}
	
		string mem = bot.GetVirtualPlayerName();
		if( mem == "[Character not dressed yet!]")
		{
			GetGame().GetCallqueue().CallLater(delayed_name_set, 512, true, entry, bot_id);
			return;
		}
		entry.m_wName.SetText(mem);
	}
	
	// Written by me
	void DeleteEntry(notnull SCR_PlayerListEntry entry, int id)
	{
		if (entry.m_wRow)
			entry.m_wRow.RemoveFromHierarchy();
		
		m_aEntries.RemoveOrdered(id);  // Is this enough?
	}
	
	
	//------------------------------------------------------------------------------------------------
	void RemoveEntry(notnull SCR_PlayerListEntry entry)
	{
		if (entry.m_wRow)
			entry.m_wRow.RemoveFromHierarchy();


		m_aEntries.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdatePlayerList( int id, Faction faction)
	{
			SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
			CreateEntry(id, editorDelegateManager, faction);

			// Get current sort method and re-apply sorting
			if (!m_Header)
				return;

			OnHeaderChanged(m_Header);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnEntryFocused(Widget w)
	{		
		if (!m_PlayerController)
			return;
		
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (!entry)
				continue;

			Widget row = entry.m_wRow;
			if (row != w)
				continue;

			m_SelectedEntry = entry;
			break;
		}
		
		if (m_SelectedEntry)
			UpdateViewProfileButton(m_SelectedEntry.m_iID);
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryFocusLost(Widget w)
	{
		UpdateViewProfileButton(0, true);
	}

	//------------------------------------------------------------------------------------------------
	// IsLocalPlayer would be better naming
	protected bool IsLocalPlayer(int id)
	{
		if (id <= 0)
			return false;

		return SCR_PlayerController.GetLocalPlayerId() == id;
	}

	
	protected bool CanOpenPlayerActionList(SCR_PlayerListEntry entry)
	{
		if (!entry)
			return false;
		
		if (m_VotingManager)
		{
			//~ Check if can vote, if yes return true
			array<EVotingType> votingTypes = {};
			m_VotingManager.GetVotingsAboutPlayer(entry.m_iID, votingTypes, true, true);
			
			//~ Check if UI info can be found
			foreach(EVotingType votingType : votingTypes)
			{
				if (m_VotingManager.GetVotingInfo(votingType))
					return true;
			}
		}
		
		//~ Can invite the player so return true
		if (m_PlayerGroupController.CanInvitePlayer(entry.m_iID))
			return true;
		
		//~ Check if Player actions have group dropdown
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupManager)
		{
			//~ No group so no drop down
			SCR_AIGroup group = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
			if (group)
			{
				array<int> requesters = {};
				group.GetRequesterIDs(requesters);
		
				if (requesters.Contains(entry.m_iID))
					return true;
			}
		}
		
		//Check if reporting is available
		if (entry.m_iID != SCR_PlayerController.GetLocalPlayerId())
			return true;
		
		//~ None of the conditions met
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateViewProfileButton(int playerId, bool forceHidden = false)
	{
		if (!m_ViewProfile)
			return;

		m_ViewProfile.SetVisible(!forceHidden && GetGame().GetPlayerManager().IsUserProfileAvailable(playerId), false);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBlockElements()
	{
		if (!m_SelectedEntry || !m_SocialComponent)
			return;
		
		//no self block
		if (m_SelectedEntry.m_iID == SCR_PlayerController.GetLocalPlayerId())
		{
			if (m_Block)
			{
				m_Block.SetVisible(false, false);
				m_Block.SetEnabled(false);
			}
			
			if (m_Unblock)
			{
				m_Unblock.SetVisible(false, false);
				m_Unblock.SetEnabled(false);
			}
			
			m_SelectedEntry.m_wBlockedIcon.SetOpacity(0);
			return;
		}
		
		bool isBlocked = m_SocialComponent.IsBlocked(m_SelectedEntry.m_iID);
		
		m_SelectedEntry.m_wBlockedIcon.SetOpacity(isBlocked);
		
		if (m_Block)
		{
			m_Block.SetVisible(!isBlocked, false);
			m_Block.SetEnabled(!isBlocked);
		}
		
		if (m_Unblock)
		{
			m_Unblock.SetVisible(isBlocked, false);
			m_Unblock.SetEnabled(isBlocked);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTabChanged(SCR_TabViewComponent comp, Widget w, int selectedTab)
	{
		if (selectedTab < 0)
			return;

		Faction faction = null;
		foreach (Faction playableFaction : m_aFactions)
		{
			if (comp.GetShownTabComponent().m_sTabButtonContent == playableFaction.GetFactionName())
				faction = playableFaction;
		}

		int lowestZOrder = int.MAX;
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (!entry.m_wRow)
				continue;

			//if the tab is the first one, it's the All tab for now
			if (comp.GetShownTab() == 0)
				entry.m_wRow.SetVisible(true);
			else if (faction == entry.m_Faction)
				entry.m_wRow.SetVisible(true);
			else
				entry.m_wRow.SetVisible(false);
		}

		if (m_Header)
			m_Header.SetCurrentSortElement(DEFAULT_SORT_INDEX, ESortOrder.ASCENDING, useDefaultSortOrder: true);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateFrequencies()
	{
		return;
		SCR_GadgetManagerComponent gadgetManager;
		IEntity localPlayer = SCR_PlayerController.GetLocalMainEntity();
		Faction localFaction;
		set<int> localFrequencies = new set<int>();
		if (localPlayer)
		{
			gadgetManager = SCR_GadgetManagerComponent.Cast(localPlayer.FindComponent(SCR_GadgetManagerComponent));
			SCR_Global.GetFrequencies(gadgetManager, localFrequencies);

			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(localPlayer.FindComponent(FactionAffiliationComponent));
			if (factionAffiliation)
				localFaction = factionAffiliation.GetAffiliatedFaction();
		}

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_Faction == localFaction)
			{
				IEntity playerEntity = IEntity.Cast(CLINTON_RoboPlayerManager.GetPlayers()[entry.m_iID].GetCurrentCharacter());
				
				if (playerEntity)
				{
					//--- ToDo: Don't extract frequencies locally; do it on server and distribute values to all clients
					gadgetManager = SCR_GadgetManagerComponent.Cast(playerEntity.FindComponent(SCR_GadgetManagerComponent));
					set<int> frequencies = new set<int>();
					SCR_Global.GetFrequencies(gadgetManager, frequencies);
					if (!frequencies.IsEmpty())
					{
						entry.m_iSortFrequency = frequencies[0];
						entry.m_wFreq.SetText(SCR_FormatHelper.FormatFrequencies(frequencies, localFrequencies));
						continue;
					}
				}
			}
			entry.m_iSortFrequency = int.MAX;
			entry.m_wFreq.SetText("-");
		}
		GetGame().GetCallqueue().CallLater(UpdateFrequencies, 1000, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddFactionPlayerCounter(Faction faction)
	{
		SCR_Faction scriptedFaction = SCR_Faction.Cast(faction);
		if (!scriptedFaction)
			return;
		
		Widget contentLayout = GetRootWidget().FindAnyWidget("FactionPlayerNumbersLayout");
		if (!contentLayout)
			return;
		
		Widget factionTile = GetGame().GetWorkspace().CreateWidgets(FACTION_COUNTER_LAYOUT, contentLayout);
		if (!factionTile)
			return;
		
		RichTextWidget playerCount = RichTextWidget.Cast(factionTile.FindAnyWidget("PlayerCount"));
		if (!playerCount)
			return;
		
		ImageWidget factionFlag = ImageWidget.Cast(factionTile.FindAnyWidget("FactionFlag"));
		if (!factionFlag)
			return;

		int x, y;
		factionFlag.LoadImageTexture(0, scriptedFaction.GetFactionFlag());	
		factionFlag.GetImageSize(0, x, y);
		factionFlag.SetSize(x, y);
		
		playerCount.SetText(scriptedFaction.GetPlayerCount().ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAddClick(SCR_ButtonBaseComponent comp)
	{		
		EditBoxWidget entry_field = EditBoxWidget.Cast(	GetRootWidget().FindAnyWidget("CLINTON_number_field"));
		
		string input = entry_field.GetText();
		// Using parser. Maybe modify to fail instead of consume
		int converted = input.ToInt();
		if(!converted)
		{
			entry_field.SetColor(Color.FromRGBA(188,0,0,255));
			return;
		}
		entry_field.SetColor(Color.Black);
		
		// Get Combo Box selections
		XComboBoxWidget factionBox = XComboBoxWidget.Cast(GetRootWidget().FindAnyWidget("CLINTON_ComboBox"));
		int faction_setting = factionBox.GetCurrentItem() -1;
		
		Widget w = GetRootWidget().FindAnyWidget("CLINTON_ComboRoot");
		SCR_ComboBoxComponent scr = SCR_ComboBoxComponent.Cast(w.GetHandler(0));
		if(!scr) Print("Wrong Handler / Controller", LogLevel.ERROR);
		int groups_setting = scr.GetCurrentIndex();
		
		int last_existing_bot_index = CLINTON_RoboPlayerManager.GetPlayers().Count();
		string bot_name;
		if(faction_setting < 0)
		{
			int j = 0;
			foreach(Faction faction : m_aFactions)
			{
				for(int i = 0; i < converted; i++)
				{
					bot_name = CLINTON_RoboPlayerManager.getInstance().add_bot( faction.GetFactionKey(), groups_setting);
					UpdatePlayerList(last_existing_bot_index + j, faction);
					j = j + 1
				}
			}
			//CLINTON_RoboPlayerManager.getInstance().add_bots_on_each_team(converted, groups_setting);
		} else {
			string factionKey = m_aFactions[faction_setting].GetFactionKey();
			for(int i = 0; i < converted; i++)
			{
				bot_name = CLINTON_RoboPlayerManager.getInstance().add_bot(factionKey, groups_setting);
				UpdatePlayerList(last_existing_bot_index + i, m_aFactions[faction_setting]);
			}
		}
		
		// Consider another way todo this
		int i = 0;
		int j = converted;
		if(faction_setting < 0) j = j * m_aFactions.Count();
		while( i < j)
		{
			//UpdatePlayerList( true, last_existing_bot_index + i);
			i = i + 1;
		}
		
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRemoveClick(SCR_ButtonBaseComponent comp)
	{		
		EditBoxWidget entry_field = EditBoxWidget.Cast(	GetRootWidget().FindAnyWidget("CLINTON_number_field"));
		
		string input = entry_field.GetText();
		// Using parser. Maybe modify to fail instead of consume
		int converted = input.ToInt();
		if(!converted)
		{
			entry_field.SetColor(Color.FromRGBA(188,0,0,255));
			return;
		}
		entry_field.SetColor(Color.Black);
		
		// Get Combo Box selections
		XComboBoxWidget factionBox = XComboBoxWidget.Cast(GetRootWidget().FindAnyWidget("CLINTON_ComboBox"));
		int faction_setting = factionBox.GetCurrentItem() -1;
		
		array<int> deleted_indexes = {};
		
		if(faction_setting < 0)
		{
			deleted_indexes = CLINTON_RoboPlayerManager.getInstance().remove_bots_on_each_team(converted, m_aEntries);
		} else {
			string factionKey = m_aFactions[faction_setting].GetFactionKey();
			deleted_indexes = CLINTON_RoboPlayerManager.getInstance().remove_bots_on_faction(factionKey, converted, m_aEntries);
		}
		foreach( int index : deleted_indexes)
		{
			//UpdatePlayerList( false, index);  // is index considering the removed items?
		}
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
		//if (m_bFiltering)
		//	OnFilter();
		//else
			Close();
	}
	/*
	//------------------------------------------------------------------------------------------------
	void SetupComboBoxDropdown(array<Faction> m_aFactions)
	{
		FrameWidget fw = FrameWidget.Cast(GetRootWidget().FindAnyWidget("Attribute_Dropdown0"));
		SCR_DropdownEditorAttributeUIComponent dropDownComp = fw.FindHandler(SCR_DropdownEditorAttributeUIComponent);
		// See SCR_DropdownEditorAttributeUIComponent.GetEntries()
	}*/
	
	protected ref array<ref string> myInfoList = {};
    protected int currentSelectedItem = 0;
    
    // -----------------------------------------------------------------------------------------------------
    array<ref string> GetInfo()
    {                
        return myInfoList;
    }
    
    // -----------------------------------------------------------------------------------------------------
    int GetCurrentSelectedItem()
    {
        return currentSelectedItem;
    }
    
    // -----------------------------------------------------------------------------------------------------
    void SetCurrentSelectedItem(int index)
    {
        currentSelectedItem = index;
        Print(currentSelectedItem);
    }    
    
    // -----------------------------------------------------------------------------------------------------
    void setupFactionPicker()
    {
        // This will initialize myInfoList with 10 strings
        myInfoList.Clear();
        int count = 10;
        for(int i = 0; i < count; i++)
        {                
            myInfoList.Insert("Info_"+i);
        }    
        
        // The selected item will be the second.
        currentSelectedItem = 1    
		// -ripped from discord
    }  
}

/*
now this sends the button info

MenuBase myMenu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.SPK_myTestGui); 
SPK_myMenuUI myMenuUI = SPK_myMenuUI.Cast(myMenu);
myMenuUI.myCallerEntity = pOwnerEntity;  // pOwnerEntity is the variable we want to be able to use in the GUI on this example.
*/