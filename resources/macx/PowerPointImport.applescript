tell application "Microsoft PowerPoint"
	launch
	set presentationCount to number of presentations
	-- see http://thesource.ofallevil.com/mac/developers/default.mspx?CTT=PageView&clr=99-21-0&target=2511850e-bf23-4a4e-a58a-078b50c6c6a11033&srcid=7830652b-fe36-4563-bedb-94aa37694b301033&ep=7
	repeat 3 times
		try
			set thePresentation to presentation "%@"
		on error
			open POSIX file "%@"
			set thePresentation to active presentation
		end try
		if thePresentation is not missing value then
			exit repeat
		else
			delay 1
		end if
	end repeat
	save thePresentation in (POSIX file "%@" as text) as save as PDF
	set slideShow to slide show view of slide show window of thePresentation
	if slideShow exists then
		exit slide show slideShow
	end if
	if number of presentations is not equal to presentationCount then
		close thePresentation
	end if
	tell application "System Events"
		set visible of process "Microsoft PowerPoint" to false
	end tell
end tell
