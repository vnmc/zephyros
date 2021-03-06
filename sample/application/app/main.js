var timeoutId = null;

function setMessage(message)
{
	$('#messages').html(message);
}

function appendMessage(message)
{
	$('#messages').html($('#messages').html() + message);
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

var fileDlgOptions = {
	initialFile: 'random.jpg',
	//initialDirectory: 'C:\\Users',
	initialDirectory: '/Users',
	//*
	filters: [
		{ description: 'Images', extensions: '*.png;*.jpg;*.jpeg' },
		{ description: 'All Files', extensions: '*.*' }
	], //*/
	folderSelectionText: 'FLDR'
};


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
				app.openURLWithBrowser('http://www.vanamco.com', browser);
			});

			$eltBrowsers.append($elt);
		});
	});

	app.onAppTerminating(function()
	{
		return;
	});

	$('.open-with-default-browser').click(function()
	{
		app.openURL('http://www.vanamco.com');
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
		app.showSaveFileDialog(fileDlgOptions, function(path)
		{
			setMessage('Path: ' + path.path);
		});
	});

	$('.open-file').click(function()
	{
		setMessage('opening file...');
		app.showOpenFileDialog(fileDlgOptions, function(path)
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

	$('#getNetworkIPs').click(function()
	{
		app.getNetworkIPs(function(ips)
		{
			setMessage(JSON.stringify(ips));
		});
	});
	
	$('#getMACAddress').click(function()
	{
		app.getMACAddress(setMessage);
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
	    app.existsFile(path, function(response)
	    {
    	    setMessage('File ' + path.path + ' exists: ' + response);	    
	    });
	});
	
	$('#isDirectory').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Checking if ' + path.path + ' is a directory...');
	    app.isDirectory(path, function(response)
	    {
    	    setMessage('File ' + path.path + ' is a directory: ' + response);	    
	    });
	});

	$('#stat').click(function()
	{
		app.stat(getParameterAsPath(), function(info)
		{
			setMessage(JSON.stringify(info));
		});
	});

	$('#makeDirectory').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Trying to create directory ' + path.path + '...');
	    app.makeDirectory(path, function(err)
	    {
	    	if (err === null)
    	    	setMessage('File ' + path.path + ' has been created.');
    	    else
    	    	setMessage('Error: ' + JSON.stringify(err));
	    });
	});

	$('#readDirectory').click(function()
	{
		app.readDirectory(getParameterAsPath(), function(err, files)
		{
			if (err === null)
				setMessage(JSON.stringify(files));
			else
    	    	setMessage('Error: ' + JSON.stringify(err));
		});
	});

	$('#readFile').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Trying to read file  ' + path.path + '...');
	    app.readFile(path, { encoding: '' }, function(err, data)
        {
            if (err === null)
        	    setMessage(data);
        	else
        	    setMessage('Could not read file. Error: ' + JSON.stringify(err));
    	 });
	});
	
	$('#writeFile').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Trying to write to file  ' + path.path + '...');
	    app.writeFile(path, 'Hey this is the file contents', {}, function(err)
	    {
	    	if (err === null)
    	    	setMessage('File ' + path.path + ' was written.');
    	    else
    	    	setMessage('Error: ' + JSON.stringify(err));
	    });
	});

	$('#writeFileBase64').click(function()
	{
	    var path = getParameterAsPath();
	    setMessage('Trying to write to file  ' + path.path + '...');
	    app.writeFile(path, 'iVBORw0KGgoAAAANSUhEUgAAAKkAAACpCAMAAABnC+0dAAAAbFBMVEVHcEz/Bwf/Cgr/Bgb/Bwf/Bwf/Bwf/Bgb/AAD/Bwf/AwP/Bwf/Bwf/AAD/Bwf/AAD/Bwf/Bwf/Bwf/Bwf/Bwf/Bwf/CAj/Bwf/Bgb/Bwf/Bwf/Bgb/Bwf/Bwf/Bwf/Bwf/CAj/Bgb/BQX/Bwcu3BALAAAAI3RSTlMA4Rkrsvf6VQb+EMFDCdkD0F7xmbmBOyOhasl6b+qrj2NMM5/IgPUAAAPqSURBVHhe7M4xDoAwDATBFCg4kjGV7QIpDf//I4ICqCHlzgP2rrwBAAAAkqrzKXL9GYqro5pSBlvCJ6v7rVn3+LIi6t3aE6o2bTHs7kGdnSYxCgJRAH4uqEhcY9bFLO/+d5x/MzWFSVA06XwHoCgsHt1teDCKA9q4yTFG3sQtByhzCOGtSruET6n9A662e8Wnki6t4KM4BHzjeIeL+5FvlIcCk90udFCv8M6qpoNLg2keRzrRug/xStjTiaaJMF610XTWNniuaeksOVUYKTQcpc8xLI85igkxyvbCkcoHhjwCjnTZYoRUcTSVwpYmfgvZ/Ncnk8YOD80JkhSOUs1pTvjfidPoFE62ihPpM/4BzpxKbeEgajmZbqxPP4luI7xV1PSQ/Htb7wmn03WBd/b08vcwHope9ngjpacyBwDkJT2leCkP6En3AICevoICr1zp7wDgTH9XvBAl9NfusFP0l+zwXM85mMpwDj2e2mnOIia57KHuKcsaT+SKsqgcww6U5oZhhtIYDAo1pdEhhtwoTyPg5vvUKQHF0QEGhJQohG1FiVZC09R2gO1Kia6wxZQohs1QIG1gqynREbaSEpVCg98W/MxOy5/5+jVsx5+5UYYSGdg6StT9zBvVO0wl5JbSa0q0hm1DiU4/U5/eYLtTojtsESWKYCs05dHZj3TRDDCkE9ycyI+pDYaklCfFkJ3cEYr8K1U6jPllD/obStNgWJ5QliTHE0ZowW87yxz0yf8hpXI81cvsocTXqCu8UMscnwhvUW54JQsoRZDhRw71hteKQOKRSq5SdYq3jNCH1PbQ/L4kgoON0P7JVpT8Ml0WcBIpfle7g6ObwI5UZEe1hrui5vfUBUaIWn5LG2GUleJ3qC1GSrXMV9RWnQTM9R2thV57W9bx07oMk2QxPyvOAK+tyt8oUHRCN2orDD+lK+AlP/IzTAFPWc9P6DN4q/Zc3r7CHK5c2hUz2Qh4QiVsVZ8woybhUlSKWa1aLuOyxcyigEsIIswurDm/YwgHAoqAuMAiqjXnta6wlJPmfPQZFpFplTSwiEyrdoWFRcFC6SQ0reoQH5Ab+jI5PiLbS00n20aLTSc7raTUTu5p5Z9OQtMqiP60c0c5CMJAEEDHqghGLIjaCBQSvP8dPcD+GDfpjsm+Q8xOOklRXBgV3ams9lCgOxl1qzXCytyQppN0OrKlk/7TpPyEsfTdynpNMBc6RTrRpdVSg0IcOLqT/t3qASJzQxqjUl8pYpSiBt4uoLPlt5Q3ENrvFG3U+AZ0AYzkeDWeQau9i9GOVb0oLqjRZR0i2K1iC6M1iYmJ1qvq8ScSfuKcc84559wHVw+eefv5E7AAAAAASUVORK5CYII=', { encoding: 'base64' }, function(err)
	    {
	    	if (err === null)
    	    	setMessage('File ' + path.path + ' was written.');
    	    else
    	    	setMessage('Error: ' + JSON.stringify(err));
	    });
	});

	$('#moveFile').click(function()
	{
		var oldPath = {
	        path: 'a.txt',
	        urlWithSecurityAccessData: '',
	        hasSecurityAccessData: false
	    };

		var newPath = {
	        path: 'b.txt',
	        urlWithSecurityAccessData: '',
	        hasSecurityAccessData: false
	    };

	    app.writeFile(oldPath, '42', {});
		app.moveFile(oldPath, newPath, function(err)
		{
			setMessage('Moving the file ' + (err === null ? 'succeeded' : 'failed with error ' + JSON.stringify(err)));
		});
	});

    $('#copyFile').click(function()
    {
        var oldPath = {
            path: 'a.txt',
            urlWithSecurityAccessData: '',
            hasSecurityAccessData: false
        };

        var newPath = {
            path: 'b.txt',
            urlWithSecurityAccessData: '',
            hasSecurityAccessData: false
        };

        app.writeFile(oldPath, '42', {}, function(err1)
        {
            if (err1)
                setMessage('Creating the file ' + (err1 === null ? 'succeeded' : 'failed with error ' + JSON.stringify(err1)));
            else
            {
                app.copyFile(oldPath, newPath, function(err2)
                {
                    setMessage('Copying the file ' + (err2 === null ? 'succeeded' : 'failed with error ' + JSON.stringify(err2)));
                });
            }
        });
    });

	$('#showOpenFileDialog').click(function()
	{
		setMessage('Trying to select file via dialog...');
		app.showOpenFileDialog(fileDlgOptions, function(path)
		{
			setMessage('File selected: ' + path.path);	
		});
	});

	$('#showOpenDirectoryDialog').click(function()
	{
		setMessage('Trying to select folder via dialog...');
		app.showOpenDirectoryDialog(fileDlgOptions, function(path)
		{
			setMessage('Folder selected: ' + path.path);	
		});
	});

	$('#showOpenFileOrDirectoryDialog').click(function()
	{
		app.showOpenFileOrDirectoryDialog(fileDlgOptions, function(path)
		{
			setMessage('File/folder selected: ' + path.path);
		});
	});

	$('#storePreferences').click(function()
	{
		setMessage('Trying to store preferences');
		var keyValueArray = getParameter().split("=");
		if (keyValueArray.length !== 2)
		{
			setMessage('Make sure to type key=value in the paramter field to try out storing preferences...');
			return;
		}

		app.storePreferences(keyValueArray[0], keyValueArray[1], function(data)
		{
			setMessage('I have been called back from storePreferences, data: ' + data);
		});
	});

	$('#getApplicationResourcesDirectory').click(function()
	{
		app.getApplicationResourcesDirectory(function(path)
		{
			setMessage(path.path);
		});
	});

	$('#loadPreferences').click(function()
	{
		var key = getParameter();
		setMessage('Trying to load preferences for key ' + key);
		
		app.loadPreferences(key, function(data)
		{
			setMessage('I have been called back from loadPreferences, data: ' + data);
		});
	});

	$('#startWatchingFiles').click(function()
	{
		var dir = getParameterAsPath();
		var extensions = ['.txt', '.tx']
		setMessage('Trying to start watching files in: ' + dir);
		
		app.startWatchingFiles(dir, extensions, 0.5);
		setMessage('Called function ok');
		
	});

	$('#stopWatchingFiles').click(function()
	{
		setMessage('Telling app to stop watching files');
		app.stopWatchingFiles();
	});

	$('#resizeMainWindow').click(function()
	{
		var sizeArr = getParameter().split('x');
		if (sizeArr.length !== 2)
			setMessage('To resize windows, specify parameter as 123x123 (e.g.)');
		else
		{
			setMessage('Resizing window to ' + sizeArr[0] + ' by ' + sizeArr[1]);
			app.setWindowSize(
				{
					width: parseInt(sizeArr[0]), 
					height: parseInt(sizeArr[1])
				},
				function(data)
				{
					setMessage('Callback was called with data: ' + JSON.stringify(data));
				}
			);
		}
	});

	$('#setMinimumWindowWidth').click(function()
	{
		setMessage('Setting minimum window dimensions to 400x400');
		app.setMinimumWindowSize({width: 400, height: 400});
		setMessage('+DONE setting minimum window dimensions to 400x400.');
	});

	$('#requestUserAttention').click(function()
	{
		setMessage('Requesting user attention');
		app.requestUserAttention();
	});

	$('#showFullScreenImage').click(function()
	{
		app.showFullScreenImage('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAMAAACdt4HsAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAyVBMVEX///8AceMAbfEAbfAAbvEAbfAAbPAAbfAAbfAAbe0AbPAAbu8AbfAAbfAAZv8AbfAAAP8AbPEAbfAAbfAAb/QAbfAAbvAAbfAAbfEAbfAAgP8AbfAAbfAAbfAAbPAAbPAAbvAAbvAAbfAAbe4AbPEAbvAAbPAAavQAcO8AgP8Abf8Abe8Ab/EAbfAAbfEAbfAAa/AAgP8AbvIAbe8AbfAAbfAAbfAAbO8AbfAAbPAAa/IAbPAAbfAAbfAAbu8AYP8AbfAAbfAAAADoM+EfAAAAQXRSTlMACUt3kKnB2vMcaLP2ZwXQAVrc0Re+M+Nb9wKKr/61hIe68D1HZnYYIAYHMTe3wrkyBDrE+PvnYSPdE0Luy0EI+mvg8b0AAAABYktHREIQ1z30AAAACXBIWXMAAA3XAAAN1wFCKJt4AAAAB3RJTUUH4AUKEgMIR5ly2QAAAexJREFUWMOlV+d6wjAMNFlklRD27J50LzYFv/9LtYmBLxROMZ/u7+kusS1bkhAABcO0bKfoukXHtkyjII6C5weh3EEYnHi66pIfyYOI/JKGvBxXJEQlLufpjaokUTVIea0uc1GvYX2jma+XstlA+lZbRy9lu3VQ3unqyRN0Owf0PX29lL19hyO+n/7D3vqP00v5bx8amvuX2cmds6hpnd8umtl8QPnTPz07v0AO9Uz+gpDLq4S9DgG9zeoyyv8bxd8Curq5WTH6yTvF3yM+VnwJ3d/+gwoY9EFARb0PPtzoR2XwBAP8lI8g/6yS/AUGRAnvSYzXt/fBxycR4JErSPD1TdLJGgLJQCBEIeQYhAWYhSmGo/F4NKQiDGES7GSa7PJ0QoSYwiLYmcqDGRFiCRuT7vrd6rg4xhYOJqub20YUG0cUMRltDHCuyqJweQYu34C9BIdn4FDHqGNgU4mkY2BRqaxjYFKXScfAoK6zhsHfdSYeFA2DgHzSNAx88lHVMPBIOt8gogtLvoFPl7Zcg3Vpg8V1vq6+5TkIiPPK+7qLaQB6W95hNi7SXywtAJ1pm1GLE/0slz9oBzItDtFkrVaI2Wmy2G0ev9Hkt7r8Zpvd7gv2wCH4Iw9/6BLssU/wB0/BHn1T8IZvBd3x/xdrydrnZYcmgAAAACV0RVh0ZGF0ZTpjcmVhdGUAMjAxNi0wNS0xMFQxODowMzowOCswMjowMAR5TEIAAAAldEVYdGRhdGU6bW9kaWZ5ADIwMTYtMDUtMTBUMTg6MDM6MDgrMDI6MDB1JPT+AAAAGXRFWHRTb2Z0d2FyZQB3d3cuaW5rc2NhcGUub3Jnm+48GgAAAABJRU5ErkJggg==');
	});

	$('#copyToClipboard').click(function()
	{
		var t = getParameter();
		app.copyToClipboard(t);
		setMessage('Copied to clipboard: "' + t + '"');
	});

	$('#getProxyForUrl').click(function()
	{
		var url = getParameter();
		setMessage('proxy for url ' + url);
		app.getProxyForURL(url, function(data)
		{
			setMessage('Proxy: ' + JSON.stringify(data));
		});
	});	


	$('#startProcess').click(function()
	{
		setMessage('Starting process...');

	    //*
		var args = getParameter().split('|');
		app.startProcess(args[0], args.slice(1), '~', function(err, exitCode, data)
		{
			alert(JSON.stringify(err));
			var msg = '<br><b>Program exited with code ' + exitCode + ', with the following output: </b><br>';
			for (var i = 0; i < data.length; i++)
				msg += data[i].text.replace('\n', '<br>');
			appendMessage(msg);
		});
		//*/

	    /*
		var t = getParameter();
		app.startProcess("/usr/local/bin/node", ["xx.js", "__111___"], "/home/administrator/tmp/stderrout", function(err, exitCode, data)
		{
			if (err)
				setMessage('Error: ' + JSON.stringify(err));
			else
			{
				var msg = '<br><b>Program exited with code ' + exitCode + ', with the following output: </b><br>';
				for (var i = 0; i < data.length; i++)
					msg += data[i].text.replace('\n', '<br>');
				appendMessage(msg);
			}
		});
		//*/

		/*
		setTimeout(function()
		{
			app.startProcess("/usr/bin/node", ["xx.js", "__222___"], "/home/administrator/tmp/stderrout", function(err, exitCode, data)
			{
				if (err)
					setMessage('Error: ' + JSON.stringify(err));
				else
				{
					var msg = '<br><b>Program exited with code ' + exitCode + ', with the following output: </b><br>';
					for (var i = 0; i < data.length; i++)
						msg += data[i].text.replace('\n', '<br>');
					appendMessage(msg);
				}
			});
		}, 0);
		//*/
	});

	$('#setMenuItemStatuses').click(function()
	{
		app.setMenuItemStatuses({
			new: (Math.random() * 4) | 0,
			select_file: (Math.random() * 4) | 0,
			open_recent: (Math.random() * 4) | 0
		});
	});

	$('#dragMe').on('mousedown', function(event)
	{
		app.beginDragFile(getParameterAsPath(), event.clientX, event.clientY, function(result, effect)
		{
			setMessage('Drag ended with result=' + result + ', effect=' + effect);
		});
	});
});

// create the app menu
var menu = [
	{
		caption: '_App',
		subMenuItems: [
			{
				caption: 'A_bout',
				menuCommandId: 'about',
				key: '1',
				keyModifiers: 2,
				image: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAMAAACdt4HsAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAyVBMVEX///8AceMAbfEAbfAAbvEAbfAAbPAAbfAAbfAAbe0AbPAAbu8AbfAAbfAAZv8AbfAAAP8AbPEAbfAAbfAAb/QAbfAAbvAAbfAAbfEAbfAAgP8AbfAAbfAAbfAAbPAAbPAAbvAAbvAAbfAAbe4AbPEAbvAAbPAAavQAcO8AgP8Abf8Abe8Ab/EAbfAAbfEAbfAAa/AAgP8AbvIAbe8AbfAAbfAAbfAAbO8AbfAAbPAAa/IAbPAAbfAAbfAAbu8AYP8AbfAAbfAAAADoM+EfAAAAQXRSTlMACUt3kKnB2vMcaLP2ZwXQAVrc0Re+M+Nb9wKKr/61hIe68D1HZnYYIAYHMTe3wrkyBDrE+PvnYSPdE0Luy0EI+mvg8b0AAAABYktHREIQ1z30AAAACXBIWXMAAA3XAAAN1wFCKJt4AAAAB3RJTUUH4AUKEgMIR5ly2QAAAexJREFUWMOlV+d6wjAMNFlklRD27J50LzYFv/9LtYmBLxROMZ/u7+kusS1bkhAABcO0bKfoukXHtkyjII6C5weh3EEYnHi66pIfyYOI/JKGvBxXJEQlLufpjaokUTVIea0uc1GvYX2jma+XstlA+lZbRy9lu3VQ3unqyRN0Owf0PX29lL19hyO+n/7D3vqP00v5bx8amvuX2cmds6hpnd8umtl8QPnTPz07v0AO9Uz+gpDLq4S9DgG9zeoyyv8bxd8Curq5WTH6yTvF3yM+VnwJ3d/+gwoY9EFARb0PPtzoR2XwBAP8lI8g/6yS/AUGRAnvSYzXt/fBxycR4JErSPD1TdLJGgLJQCBEIeQYhAWYhSmGo/F4NKQiDGES7GSa7PJ0QoSYwiLYmcqDGRFiCRuT7vrd6rg4xhYOJqub20YUG0cUMRltDHCuyqJweQYu34C9BIdn4FDHqGNgU4mkY2BRqaxjYFKXScfAoK6zhsHfdSYeFA2DgHzSNAx88lHVMPBIOt8gogtLvoFPl7Zcg3Vpg8V1vq6+5TkIiPPK+7qLaQB6W95hNi7SXywtAJ1pm1GLE/0slz9oBzItDtFkrVaI2Wmy2G0ev9Hkt7r8Zpvd7gv2wCH4Iw9/6BLssU/wB0/BHn1T8IZvBd3x/xdrydrnZYcmgAAAACV0RVh0ZGF0ZTpjcmVhdGUAMjAxNi0wNS0xMFQxODowMzowOCswMjowMAR5TEIAAAAldEVYdGRhdGU6bW9kaWZ5ADIwMTYtMDUtMTBUMTg6MDM6MDgrMDI6MDB1JPT+AAAAGXRFWHRTb2Z0d2FyZQB3d3cuaW5rc2NhcGUub3Jnm+48GgAAAABJRU5ErkJggg=='
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
				menuCommandId: 'new',
				key: 'n',
				keyModifiers: 2
			},
			{
				caption: 'Open',
				subMenuItems: [
					{
						caption: 'Select File',
						menuCommandId: 'select_file',
						key: 'f11',
						keyModifiers: 7
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
];
app.createMenu(menu);

/*
setTimeout(function()
{
    menu.push({
        caption: '_New Menu',
        subMenuItems: [
            { caption: 'M1', menuCommandId: 'M1' },
            { caption: 'M2', menuCommandId: 'M2' }
        ]
    });
    app.createMenu(menu);
}, 50000);*/

app.createTouchBar([
	{ id: 'hello', caption: 'Hello' },
	{ id: 'world', caption: 'World', image: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAMAAACdt4HsAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAyVBMVEX///8AceMAbfEAbfAAbvEAbfAAbPAAbfAAbfAAbe0AbPAAbu8AbfAAbfAAZv8AbfAAAP8AbPEAbfAAbfAAb/QAbfAAbvAAbfAAbfEAbfAAgP8AbfAAbfAAbfAAbPAAbPAAbvAAbvAAbfAAbe4AbPEAbvAAbPAAavQAcO8AgP8Abf8Abe8Ab/EAbfAAbfEAbfAAa/AAgP8AbvIAbe8AbfAAbfAAbfAAbO8AbfAAbPAAa/IAbPAAbfAAbfAAbu8AYP8AbfAAbfAAAADoM+EfAAAAQXRSTlMACUt3kKnB2vMcaLP2ZwXQAVrc0Re+M+Nb9wKKr/61hIe68D1HZnYYIAYHMTe3wrkyBDrE+PvnYSPdE0Luy0EI+mvg8b0AAAABYktHREIQ1z30AAAACXBIWXMAAA3XAAAN1wFCKJt4AAAAB3RJTUUH4AUKEgMIR5ly2QAAAexJREFUWMOlV+d6wjAMNFlklRD27J50LzYFv/9LtYmBLxROMZ/u7+kusS1bkhAABcO0bKfoukXHtkyjII6C5weh3EEYnHi66pIfyYOI/JKGvBxXJEQlLufpjaokUTVIea0uc1GvYX2jma+XstlA+lZbRy9lu3VQ3unqyRN0Owf0PX29lL19hyO+n/7D3vqP00v5bx8amvuX2cmds6hpnd8umtl8QPnTPz07v0AO9Uz+gpDLq4S9DgG9zeoyyv8bxd8Curq5WTH6yTvF3yM+VnwJ3d/+gwoY9EFARb0PPtzoR2XwBAP8lI8g/6yS/AUGRAnvSYzXt/fBxycR4JErSPD1TdLJGgLJQCBEIeQYhAWYhSmGo/F4NKQiDGES7GSa7PJ0QoSYwiLYmcqDGRFiCRuT7vrd6rg4xhYOJqub20YUG0cUMRltDHCuyqJweQYu34C9BIdn4FDHqGNgU4mkY2BRqaxjYFKXScfAoK6zhsHfdSYeFA2DgHzSNAx88lHVMPBIOt8gogtLvoFPl7Zcg3Vpg8V1vq6+5TkIiPPK+7qLaQB6W95hNi7SXywtAJ1pm1GLE/0slz9oBzItDtFkrVaI2Wmy2G0ev9Hkt7r8Zpvd7gv2wCH4Iw9/6BLssU/wB0/BHn1T8IZvBd3x/xdrydrnZYcmgAAAACV0RVh0ZGF0ZTpjcmVhdGUAMjAxNi0wNS0xMFQxODowMzowOCswMjowMAR5TEIAAAAldEVYdGRhdGU6bW9kaWZ5ADIwMTYtMDUtMTBUMTg6MDM6MDgrMDI6MDB1JPT+AAAAGXRFWHRTb2Z0d2FyZQB3d3cuaW5rc2NhcGUub3Jnm+48GgAAAABJRU5ErkJggg==' },
	{ id: 'long', caption: 'Lorem Ipsum', backgroundColor: '#5050ff', color: '#ff50ff' },
	{
		items: [
			{ id: '_a', caption: 'Alpha' },
			{ id: '_b', caption: 'Beta' },
			{ id: '_c', caption: 'Gamma' },
			{ id: '_d', caption: 'Delta' },
			{ id: '_e', caption: 'Epsilon' },
			{ id: '_z', caption: 'Zeta' },
			{ id: '_th', caption: 'Theta' }
		]
	}
]);

// react to the menu
app.onMenuCommand(function(commandId)
{
	setMessage('Menu command: ' + commandId);
});

app.onTouchBarAction(function(id)
{
	setMessage('Touch bar identifier: ' + id);
});

// onFileChanged: (callback: (paths: string[]) => void) => void
app.onFileChanged(function(paths)
{
	setMessage('File(s) was/were changed...paths = ' + JSON.stringify(paths));
});
