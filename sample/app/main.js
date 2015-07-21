$(document).ready(function()
{
	// find all the browsers installed on the system
	app.getBrowsers(function(browsers)
	{
		var $eltBrowsers = $('#browsers');

		// add a list entry for each browser found on the syste
		browsers.forEach(function(browser)
		{
			var $elt = $('<li><img src="' + browser.image + '"/> ' + browser.name + '</li>');

			// open a webpage in that browser when the list entry is clicked
			$elt.click(function()
			{
				app.openUrl('http://www.vanamco.com', browser);
			});

			$eltBrowsers.append($elt);
		});
	});
});
