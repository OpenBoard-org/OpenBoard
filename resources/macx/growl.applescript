tell application "System Events"
	set isRunning to Â
		(count of (every process whose name is "GrowlHelperApp")) > 0
end tell

if isRunning then
	
	tell application "GrowlHelperApp"
		-- Make a list of all the notification types 
		-- that this script will ever send:
		set the allNotificationsList to Â
			{"Uniboard Update Install"}
		
		set the enabledNotificationsList to Â
			{"Uniboard Update Install"}
		
		register as application Â
			"Uniboard" all notifications allNotificationsList Â
			default notifications enabledNotificationsList Â
			icon of application "Uniboard"
		
		notify with name Â
			"Uniboard Update Install" title Â
			"Uniboard" description Â
			"Uniboard is installing a new version." application name "Uniboard"
		
	end tell
end if