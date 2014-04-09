// A mock implementation in JavaScript of your native API.
// Allows you to test your HTML5 app in a browser.

var app = {
	onMenuCommand: function() {},
	onAppTerminating: function() {},

	myFunction: function(firstNumber, secondNumber, callback)
	{
		callback(firstNumber + secondNumber);
	},

	showOpenFileDialog: function(callback)
	{
		callback('/my/file.txt');
	},

	showOpenDirectoryDialog: function(callback)
	{
		callback('/my/path/');
	},

	showInFileManager: function() {}
};