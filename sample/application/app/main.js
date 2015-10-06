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

	$('.compute-fib').click(function()
	{
	    var n = parseInt($('.fib-number').val(), 10);
	    app.fibonacci(n, function(result)
	    {
	        $('.fib-result').html(result);
	    });        
	});

	var contextMenuHandle = null;
	app.createContextMenu(
		[
			{
				caption: 'One',
				menuCommandId: 'the-one'
			},
			{
				caption: 'Two',
				menuCommandId: 'another-one'
			},
			{
				caption: 'More',
				subMenuItems: [
					{
						caption: 'A',
						menuCommandId: 'a'
					},
					{
						caption: 'B',
						menuCommandId: 'b'
					}
				]
			}
		],
		function(handle) { contextMenuHandle = handle; }
	);
	$('.context-menu').click(function()
	{
		var offs = $('.context-menu').offset();
		app.showContextMenu(contextMenuHandle, offs.left, offs.top + 30, function(commandId)
		{
			$('#messages').html('Context menu command: ' + commandId);
			setTimeout(function()
			{
				$('#messages').html('');
			}, 2000);
		});
	});
});

// create the app menu
app.createMenu([
	{
		caption: '_App',
		subMenuItems: [
			{
				caption: 'A_bout',
				menuCommandId: 'about',
				key: '1',
				keyModifiers: 2
			},
			{
				caption: '_Quit',
				menuCommandId: 'terminate',
				key: 'q',
				keyModifiers: 2
			}
		]
	},
	{
		caption: '_File',
		subMenuItems: [
			{
				caption: 'New',
				menuCommandId: 'new'
			}
		]
	},
	{
		caption: '_Edit',
		subMenuItems: [
			{
				caption: '_Copy',
				key: 'c',
				keyModifiers: 2,
				systemCommandId: 'copy:'
			},
			{
				caption: '_Paste',
				key: 'v',
				keyModifiers: 2,
				systemCommandId: 'paste:'
			}
		]
	}
]);

// react to the menu
app.onMenuCommand(function(commandId)
{
	$('#messages').html('Menu command: ' + commandId);
	setTimeout(function()
	{
		$('#messages').html('');
	}, 2000);
});
