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
				app.openURL('http://www.vanamco.com', browser);
			});

			$eltBrowsers.append($elt);
		});
	});

	app.onMenuCommand(function(commandId)
	{
		$('body').append('<div>Menu "' + commandId + '" selected.</div>');
	});

	$('.compute-fib').click(function ()
	{
	    var n = parseInt($('.fib-number').val(), 10);
	    app.fibonacci(n, function(result)
	    {
	        $('.fib-result').html(result);
	    });        
	});
});


app.createMenu([
	{
		caption: 'App',
		subMenuItems: [
			{
				caption: 'About',
				menuCommandId: 'about'
			}
		]
	},
	{
		caption: 'File',
		subMenuItems: [
			{
				caption: 'New',
				menuCommandId: 'new'
			}
		]
	},
	{
		caption: 'Edit',
		subMenuItems: [
			{
				caption: 'Copy',
				key: 'c',
				keyModifiers: 1,
				systemCommandId: 'copy:'
			},
			{
				caption: 'Paste',
				key: 'v',
				keyModifiers: 1,
				systemCommandId: 'paste:'
			}
		]
	}
]);