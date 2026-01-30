function loadImage(theURL)
{
	theImage     = new Image;
	theImage.src = theURL;
}


function navSelect(theID)
{
	document.getElementById(theID).setAttribute("class", "selected");
}


var prevOnLoad = window.onload;
window.onload = function()
{
	if (prevOnLoad)
		prevOnLoad();


	// Preload images
	if (document.domain != "")
		{
		loadImage("/src/img/nav_apps_hover.png");
		loadImage("/src/img/nav_apps_selected.png");
		loadImage("/src/img/nav_projects_hover.png");
		loadImage("/src/img/nav_projects_selected.png");
		loadImage("/src/img/nav_contact_hover.png");
		loadImage("/src/img/nav_contact_selected.png");
		loadImage("/src/img/nav_dev_hover.png");
		loadImage("/src/img/nav_dev_selected.png");
		}
}

