var timeoutId = null;

function setMessage(message)
{
	$('#messages').html(message);

/*	if (timeoutId !== null)
		clearTimeout(timeoutId);

	timeoutId = setTimeout(function()
	{
		$('#messages').html('');
		timeoutId = null;
	}, 2000);*/
}

function getParameter()
{
    return $('#param').val();
}

function getParameterAsPath()
{
    return {
	    path: getParameter(),
	    urlWithSecurityAccessData: '',
	    hasSecurityAccessData: false
	};
}


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
            alert('browser: ' + JSON.stringify(browser));
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
				menuCommandId: 'another-one',
				image: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAjFJREFUeNqUkk1oE0EUx/+z2WTz1dpN0hoLhWoVwYPmpO3JStVT8SR4kAhFFEuDFE8eBBE8S/woERWsOXnwolCoFAIFv/ASoSJIhSRqbUybTZpm87Hd3XEmBk1MWuoP3s4sb/7/x5t5oJTif+LlTrej8Z/wz6y/4yiAcRanWWgs3rKInLh38xlbMRe6FqznA6LT7tBLlYV6PkJmetxDDp/8JhAag9vfDWaL6noRX2OvkZydv80O2naPHh/vGx6E5HZxP+iVCuJTUeQXE6PkRU9H9MjV8WCnbAetqAAhIFYJxC0jGXsHgxkOHDsMqq6B6lqtABFtKOsiXl0Pz4imIARdTmAjl8FfikBBQe/gQdajAW05gWZUiJILTDskCk4HqitLrDDBvxBNAzV0wDRacoaqwjCpIZbVMtT0T9jq/TWzjs0oKCVsULwXdYM++PF56eKuvf5a/9vl26c0ux9Mkye+HQFRssYPHPLB2eXalji7XMDiQmb6bOTGmHhuJf/hsa9r8mM8E963vxNOl7SlWMmWkfhS4I8Rqd0THyTOQ58cEq2WuwN9EpwOoa04X9CR/F7lEzh8YTU332TAue+Vz1sswqM9vSJc9maTfNFAKq1z8cilbC7256UaDThTXjlosZBof7cAt/33pa6VKFKrJqhJRyYaxG0NOHe8njOCQJ72yxQmS6dyhFc+eTmrzLXMSjsDTtjjOcWG6znfc/Gk0ire0oBzy+Ox8fWKomibnfklwAC7jBaZff2rbwAAAABJRU5ErkJggg=='
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
			setMessage('Context menu command: ' + commandId);
		});
	});

	$('.save-file').click(function()
	{
		app.showSaveFileDialog(function(path)
		{
			setMessage('Path: ' + path.path);
		});
	});

	$('.open-file').click(function()
	{
	setMessage('opening file...');
		app.showOpenFileDialog(function(path)
		{
			setMessage('Path: ' + path.path);
		});
	});

	$('.remove-menuitem').click(function()
	{
		app.removeMenuItem('enter_license');
		app.removeMenuItem('open_recent');
	});
	
	$('#getComputerName').click(function()
	{
	    app.getComputerName(function(computerName)
        {
           setMessage('Computer Name: ' +  computerName); 
        });
	});
	
	$('#showInFileManager').click(function()
	{
	    var p = getParameter();
	    setMessage('Open ' + p + ' in file manager');
	    var path = {
	        path: p,
	        urlWithSecurityAccessData: '',
	        hasSecurityAccessData: false
	    };
	    app.showInFileManager(path);
	});
	
	$('#existsFile').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Checking if ' + path.path + ' exists...');
	    app.existsFile(path, function(response) {
    	    setMessage('File ' + path.path + ' exists: ' + response);	    
	    });
	});
	
	$('#isDirectory').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Checking if ' + path.path + ' is a directory...');
	    app.isDirectory(path, function(response) {
    	    setMessage('File ' + path.path + ' is a directory: ' + response);	    
	    });
	});
	
	$('#makeDirectory').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Trying to create directory ' + path.path + '...');
	    app.makeDirectory(path, function(response) {
    	    setMessage('File ' + path.path + ' has been created: ' + response);	    
	    });
	});
	
	$('#readFile').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Trying to read file  ' + path.path + '...');
	    app.readFile(path, { encoding: '' }, function(data)
        {
            if (data !== null)
        	    setMessage(data);	    
        	else
        	    setMessage('Could not read file ' + path.path);
    	 });
	});
	
	$('#writeFile').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Trying to write to file  ' + path.path + '...');
	    app.writeFile(path, "Hey this is the file contents");
	});

	$('#showOpenFileDialog').click(function()
	{
		setMessage('Trying to select file via dialog...');
		app.showOpenFileDialog(function(path) {
			setMessage('File selected: ' + path.path);	
		});
	});

	$('#showOpenDirectoryDialog').click(function()
	{
		setMessage('Trying to select folder via dialog...');
		app.showOpenDirectoryDialog(function(path) {
			setMessage('Folder selected: ' + path.path);	
		});
	});


	$('#storePreferences').click(function()
	{
		setMessage('Trying to store preferences');
		var keyValueArray = getParameter().split("=");
		if(keyValueArray.length !== 2)
		{
			setMessage('Make sure to type key=value in the paramter field to try out storing preferences...');
			return;
		}

		app.storePreferences(keyValueArray[0], keyValueArray[1], function(data){
			setMessage('I have been called back from storePreferences, data: ' + data);
		});
		
	});

	$('#loadPreferences').click(function()
	{
		var key = getParameter();
		setMessage('Trying to load preferences for key ' + key);
		
		app.loadPreferences(key, function(data){
			setMessage('I have been called back from loadPreferences, data: ' + data);
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
			{ caption: '-' },
			{
				caption: 'Enter License Key',
				menuCommandId: 'enter_license'
			},
			{ caption: '-' },
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
			},
			{
				caption: 'Open',
				subMenuItems: [
					{
						caption: 'Select File',
						menuCommandId: 'select_file'
					},
					{
						caption: 'Open Recent',
						menuCommandId: 'open_recent'
					}
				]
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
	setMessage('Menu command: ' + commandId);
});
