/*******************************************************************************
 * Copyright (c) 2015 Vanamco 
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * Matthias Christen, Vanamco AG
 * Florian Müller, Vanamco AG
 *******************************************************************************/


declare module NativeInterface
{
	export interface INative
	{
		///////////////////////////////////////////////////////////////////////
		// Events

		/**
		 * The callback function will be invoked when files or URLs are dropped
		 * on the dock icon.
		 *
		 * @param callback
		 *   Function called with the files or URLs dropped on the dock icon as
		 *   argument.
		 */
		onAddURLs: (callback: (urls: IPath[]) => void) => void;

		/**
		 * The callback function will be invoked when a menu item in the native
		 * application menu is clicked.
		 *
		 * On Mac, in the inspector in XCode, set the menu item class of a menu
		 * item that you want to handle from your JavaScript app, to
		 * ZPYMenuItem and give it a user defined runtime attribute of "String"
		 * with key path "commandId" and a value that will be received by the
		 * callback.
		 *
		 * On Windows, map your menu command IDs (menuID) by using
		 * Zephyros::SetMenuIDForCommand(commandId, menuID).
		 *
		 * @param callback
		 *   Function called with the commandId of the menu item that was
		 *   selected.
		 */
		onMenuCommand: (callback: (commandId: string) => void) => void;

		/**
		 * The callback function will be called when file watching has been
		 * started on a directory, and a file in that directory has changed.
		 *
		 * @param callback
		 *   Function called with an array of IFileChange objects, each of
		 *   which identifies a file that has changed (was deleted or
		 *   modified).
		 */
		onFileChanged: (callback: (fileChanges: IFileChange[]) => void) => void;

        /**
         * The callback function will be called when the user clicked a URL
         * with a custom protocol handled by your Zephyros app.
         *
         * @param callback
         *   Function called a custom protocol URL was clicked with the URL as
         *   argument.
         *   
         */
        onCustomURL: (callback: (url: string) => void) => void;

		/**
		 * The callback function will be invoked when the app is about to
		 * terminate.
		 *
		 * @param callback
		 *   Function called when the app is about to terminate. It doesn't
		 *   receive any arguments.
		 */
		onAppTerminating: (callback: () => void) => void;


		///////////////////////////////////////////////////////////////////////
		// UI Commands

		/**
		 */
		createMenu: (menuItems: IMenuItem[]) => void;

		/**
		 * Call this function to enable/disable or check/uncheck menu items of
		 * the application's native menu bar.
		 *
		 * @param items
		 *   An IMenuItemFlags object defining which menu items' states to
		 *   modify. "items" is basically a hash with the menu items' IDs as
		 *   keys and bitwise OR combination of the constants
		 *     ENABLED = 1
		 *     CHECKED = 2
		 *   as values (i.e., a value of 0 means disabled and unchecked, 1
		 *   means enabled and unchecked, 2 is disabled and checked, 3 is
		 *   enabled and checked).
		 */
		setMenuItemStatuses: (items: IMenuItemFlags) => void;

		/**
		 * createContextMenu creates a new context menu and returns an opaque
		 * handle in the callback function once it is created. This handle is
		 * passed to showContextMenu to show the context menu at location
		 * (x,y).
		 *
		 * @param menuItems
		 *   An array of menu items (IMenuItem objects) making up the context
		 *   menu.
		 *
		 * @param callback
		 *   Callback called with an opaque menu handle to be passed to a
		 *   subsequent call to "showContextMenu".
		 */
		createContextMenu: (menuItems: IMenuItem[], callback: (menuHandle: string) => void) => void;

		/**
		 * The callback function to showContextMenu is called as soon as the
		 * user has clicked a menu item (or has made the menu disappear),
		 * which gets passed the menuCommandId as argument or the empty string
		 * if no menu item was selected.
		 *
		 * @param menuHandle
		 *   The handle of the menu to display. Call "createContextMenu" prior
		 *   to calling "showContextMenu" to obtain the menu handle.
		 *
		 * @param x
		 *   The x-coordinate of the position at which the context menu will be
		 *   displayed.
		 *
		 * @param y
		 *   The y-coordinate of the position at which the context menu will be
		 *   displayed.
		 *
		 * @param callback
		 *   Callback function called when the user selects a menu item of the
		 *   context menu.
		 */
		showContextMenu: (menuHandle: string, x: number, y: number, callback: (menuCommandId: string) => void) => void;

        /**
         * Copies the text "text" to the clipboard.
         *
         * @param text
         *   The text to copy to the clipboard.
         */
        copyToClipboard: (text: string) => void;


		///////////////////////////////////////////////////////////////////////
		// Loading/Saving Preferences Commands

		/**
		 * Loads application preferences (any JSON data, which have previously
		 * been stored using "storePreferences").
		 *
		 * On Mac, the preferences are saved in the file
		 * ~/Library/Preferences/{app-bundle-id}.plist.
		 * On Windows, the preferences are written to the registry under the
		 * key HKCU\{reg-key}, where {reg-key} is the key you set using
		 * Zephyros::SetWindowsInfo in the native app.
		 *
		 * @param key
		 *   A key under which to store the preferences. The key enables you to
		 *   store different sets of preferences.
		 *
		 * @param callback
		 *   Callback invoked with the data once they have been loaded.
		 */
		loadPreferences: (key: string, callback: (preferences: any) => void) => void;

		/**
		 * Stores application preferences.
		 *
		 * On Mac, the preferences are saved in the file
		 * ~/Library/Preferences/{app-bundle-id}.plist.
		 * On Windows, the preferences are written to the registry under the
		 * key HKCU\{reg-key}, where {reg-key} is the key you set using
		 * Zephyros::SetWindowsInfo in the native app.
		 *
		 * @param key
		 *   A key under which to store the preferences. The key enables you to
		 *   store different sets of preferences.
		 *
		 * @param preferences
		 *   The preferences to store. Can be an arbitrary JSON object.
		 */
		storePreferences: (key: string, preferences: any) => void;

		/**
		 * Loads the settings for the in-app updater.
		 *
		 * @param callback
		 *   Callback function called with an IUpdaterSettings object.
		 */
		getUpdaterSettings: (callback: (settings: IUpdaterSettings) => void) => void;

		/**
		 * Saves the settings for the in-app updater.
		 *
		 * @param settings
		 *   The IUpdaterSettings object containing the updater settings.
		 */
		setUpdaterSettings: (settings: IUpdaterSettings) => void;


		///////////////////////////////////////////////////////////////////////
		// Browser Commands

		/**
		 * Finds all browsers installed on the system and calls "callback" with
		 * the result.
		 *
		 * @param callback
		 *   Callback invoked with the result, an array of IBrowser objects.
		 */
		getBrowsers: (callback: (browsers: IBrowser[]) => void) => void;

		/**
		 * Finds the default browser installed on the system.
		 *
		 * @param callback
		 *   Callback invoked with an IBrowser object as argument representing
		 *   the default browser.
		 */
		getDefaultBrowser: (callback: (browser: IBrowser) => void) => void;

		/**
		 * Opens the URL "url" in the browser referenced by "browser".
		 *
		 * @param url
		 *   The URL to open.
		 *
		 * @param browser
		 *   An IBrowser object (which can be retrieved by calling
		 *   "getBrowsers" or "getDefaultBrowser") representing the browser
		 *   in which the URL will be opened.
		 */
		openURL: (url: string, browser: IBrowser) => void;

		/** 
		 * Tries to determine the browser (represented by an IBrowser object)
		 * corresponding to the user agent "userAgent".
		 *
		 * @param userAgent
		 *   An IUserAgent object with information about the user agent.
		 *
		 * @param callback
		 *   Callback invoked with the browser found for the user agent
		 *   "userAgent" or null if no locally installed browser for that
		 *   user agent could be found.
		 */ 
		getBrowserForUserAgent: (userAgent: IUserAgent, ca llback: (browser: IBrowser) => void) => void;


		///////////////////////////////////////////////////////////////////////
		// File System Commands

		/**
		 * Shows the OS dialog to open an existing file.
		 *
		 * @param callback
		 *   Callback invoked when the user selected a file to open. The file
		 *   is passed as an IPath object to the callback. If the user canceled
		 *   the dialog, null is passed to the callback.
		 */
		showOpenFileDialog: (callback: (path: IPath) => void) => void;

		/**
		 * Shows the OS dialog to save a file.
		 *
		 * @param callback
		 *   Callback invoked when the user entered a filename to safe. The
		 *   file is passed as an IPath object to the callback. null is passed
		 *   if the dialog was canceled by the user.
		 */
		showSaveFileDialog: (callback: (path: IPath) => void) => void;

		/**
		 * Shows the OS dialog to select a directory.
		 *
		 * @param callback
		 *   Callback invoked with the directory selected by the user as an
		 *   IPath object. If the dialog was canceled, null is passed.
		 */
		showOpenDirectoryDialog: (callback: (path: IPath) => void) => void;

		/**
		 * Shows a dialog allowing to select either a file or a directory.
		 *
		 * @param callback
		 *   Callback invoked when a file or a directory was selected by the
		 *   user. The path to the file or directory is passed as a IPath
		 *   object to the callback, or null if the dialog was canceled.
		 */
        showOpenFileOrDirectoryDialog: (callback: (path: IPath) => void) => void;

        /**
		 * Reveals the file or directory at "path" in the file manager (Finder
		 * on Mac OS X or Exporer in Windows).
		 *
		 * @param path
		 *   The path to the directory or file to reveal.
         */
		showInFileManager: (path: IPath) => void;

		/**
		 * Retrieves the path to the user's home directory.
		 *
		 * @param callback
		 *   Callback called with the path to the user's home directory.
		 */
		getHomeDirectory: (callback: (path: IPath) => void) => void;

		/**
		 * Retrieves the name of the computer on which the app is run.
		 *
		 * @param callback
		 *   Callback called with the computer name as argument.
		 */
        getComputerName: (callback: (computerName: string) => void) => void;

		/**
		 * Call this function after a drop event has occurred on the
		 * document.body to get the paths to the dropped files and URLs.
		 *
		 * @param callback
		 *   Callback invoked with an array of IPath objects containing the
		 *   paths and URLs which were dropped onto the app.
		 */
		getDroppedURLs: (callback: (paths: IPath[]) => void) => void;

        /**
         * Call this function to get the custom URLs that were passed when the
         * app was started (if any). If any URLs were passed, the event
         * handlers registered with "onCustomURL" will be invoked.
         */
        getCustomURLs: () => void;

		/**
		 * Call this function to start accessing a file resource (a "security
		 * scoped resource") – used for accessing files in sandboxed mode on
		 * Mac OS X. When done with the actual file operation, make sure to
		 * call "stopAccessingPath".
		 *
		 * Note: "readFile" and "writeFile" do this already, so there is no
		 * need to call this function prior to calling these functions.
		 *
		 * @param path
		 *   The path of the file or directory to access.
		 *
		 * @param callback
		 *   Callback invoked when it is safe to access the file specified in
		 *   "path" or if an error occurred granting access rights.
		 */
		startAccessingPath: (path: IPath, callback: (success: boolean) => void) => void;

		/**
		 * Call this function to stop accessing a file resource (a "security
		 * scoped resource"), i.e., after you're done with a file operation.
		 *
		 * @param path
		 *   The path of the file or directory that was accessed.
		 */
		stopAccessingPath: (path: IPath) => void;

		/**
		 * Determines if the file at path exists.
		 *
		 * @param path
		 *   The path of the file to test.
		 *
		 * @param callback
		 *   Callback invoked with a boolean flag indicating if the file
		 *   exists.
		 */
		existsFile: (path: IPath, callback: (exists: boolean) => void) => void;

        /**
         * Checks if the path at "path" is a directory.
         *
         * @param path
         *   The path to check.
         *
         * @param callback
         *   Callback invoked with the result, a boolean specifying if the
         *   node at "path" is a directory (if it exists at all).
         */
        isDirectory: (path: IPath, callback: (isDir: boolean) => void) => void;

        /**
         * Creates a directory. The directory is created recursively, i.e., all
         * the directories along the path are created if they don't exist yet.
         *
         * @param path
         *   The absolute path to the directory to create.
         *
         * @param callback
         *   Callback called when the directory was created or an error
         *   occurred during creation.
         */
        makeDirectory: (path: IPath, callback: (success: boolean) => void) => void;

		/**
		 * Reads the contents of the file at "path".
		 *
		 * Note: if provided in the path, the function starts and stops
		 * accessing a security scoped resource on Mac OS X, so there is no
		 * need to call startAccessingPath prior to calling this function.
		 *
		 * @param path
		 *   The path identifying the file to read.
		 *
		 * @param options
		 *   Read options.
		 *
		 * @param callback
		 *   Callback invoked with the contents of the file as a string.
		 */
		readFile: (path: IPath, options: IReadFileOptions, callback: (contents: string) => void) => void;

		/**
		 * Writes "contents" to a file located at "path".
		 *
		 * @param path
		 *   The location of the file.
		 *
		 * @param contents
		 *   The contents to write to the file.
		 */
		writeFile: (path: IPath, contents: string) => void;

		/**
		 * Deletes all the files described by "path". The file part of "path"
		 * can contain wildcards (*, ?).
		 * 
		 * @param path
		 *   The path of the files to delete. The last path component can
		 *   contain wildcard characters (*, ?) to identify multiple files.
		 *
		 * @param relativeFilenames
		 *   Optionally, you can specify the file name(s), which are treated
		 *   relatively to "path". Can contain wildcard characters (*, ?).
		 */
		deleteFiles: (path: IPath, relativeFilenames?: string) => void;

		/**
		 * Starts watching the files in the directory at "path" and its sub-
		 * directories for file changes (creations, removals, modifications).
		 *
		 * After calling this function, any file changes will be reported to
		 * the event listeners added to the "onFileChanged" event.
		 *
		 * The changes can be filtered by file extension. If the "extensions"
		 * array is not empty, only the files having one of the file extensions
		 * in the "extensions" array are reported. The extensions in the
		 * array have to start with a dot ".".
		 *
		 * Only one directory can be watched at a time.
		 * If this function is called while file watching is already active,
		 * the previous directory will no longer be watched.
		 *
		 * @param path
		 *   The path to the directory to be watched for file changes.
		 *
		 * @param extensions
		 *   An array of file extensions starting with a dot ".".
		 *   Only changes to files matching the extensions in this array will
		 *   be reported. If the array is empty, all files are watched.
		 */
		startWatchingFiles: (path: IPath, extensions: string[]) => void;

		/**
		 * Stops file watching which was previously started by calling
		 * "startWatchingFiles".
		 */
		stopWatchingFiles: () => void;

		/**
		 * Retrieves the path to the temporary directory on the system.
		 *
		 * @param callback
		 *   Callback called with the path to the temporary directory.
		 */
		getTemporaryDirectory: (callback: (path: IPath) => void) => void;

		/**
		 * Retrieves the path to the directory containing the app's resources.
		 * On Mac, this is the "Resources" directory within the app bundle.
		 * On Windows, this is the directory "res" at the location of the app's
		 * executable.
		 *
		 * @param callback
		 *   Callback called with the app's resources path.
		 */
		getApplicationResourcesDirectory: (callback: (path: IPath) => void) => void;

		/**
		 * Starts the process with path "executablePath" and arguments "args"
		 * in the directory "cwd".
		 * The callback is invoked as soon as the process has terminated.
		 * If the process couldn't be started, the callback is called with
		 * null as "exitCode".
		 *
		 * Performs globbing on executablePath, so e.g.,
		 * "~/.gem/ruby/*\bin/sass" will resolve to something like
		 * "/Users/myusername/.gem/ruby/2.0.0/bin/sass".
		 *
		 * @param executablePath
		 *   The path to the executable.
		 *
		 * @param args
		 *   The command line arguments to the executable.
		 *
		 * @param cwd
		 *   The current working directory for the executable.
		 *
		 * @param callback
		 *   Callback called when the process launched has terminated. The
		 *   callback is passed the exit code of the process and an array of
		 *   IOutputStreamData objects containing the contents of stdout and
		 *   stderr.
		 */
		startProcess: (
			executablePath: string, 
			args: string[], 
			cwd: string, 
			callback: (exitCode: number, output: IOutputStreamData[]) => void) => void;


		///////////////////////////////////////////////////////////////////////
		// Networking

		/**
		 * Retrieves the IPv4 addresses of all network adapters of the
		 * computer.
		 *
		 * @param callback
		 *   Callback called with an array of IPv4 addresses.
		 */
		getNetworkIPs: (callback: (ips: string[]) => void) => void;

		/**
		 * Retrieves the MAC address of the main network interface on the
		 * computer.
		 *
		 * @param callback
		 *   Callback invoked with the MAC address of the computer's main
		 *   network adapter.
		 */
		getMACAddress: (callback: (macAddress: string) => void) => void;

		/**
		 * jQuery-compatible ajax method that can be used for cross-domain
		 * requests (only available in the Mac OS X WebView version of
		 * Zephyros since no XHR requests are supported there).
		 *
		 * @param options
		 *   Cf. http://api.jquery.com/jquery.ajax/
		 */
		ajax: (options: IAJAXOptions) => void;

		/**
		 * Queries the system for a proxy URL to be used when accessing the URL
		 * "url".
		 *
		 * @param url
		 *   The URL for which to obtain the system's proxy configuration.
		 *
		 * @param callback
		 *   The system's proxy configuration.
		 */
		getProxyForURL: (url: string, callback: (proxyConfig: IProxyConfig) => void) => void;


		///////////////////////////////////////////////////////////////
		// Windows, Notifications

		/**
		 * Sets the size of the app's window.
		 *
		 * @param size
		 *   The new size of the window.
		 *
		 * @param callback
		 *   Callback called as soon as the window was resized.
		 *   Mac OS X uses an animation to resize the window; the callback will
		 *   be called as soon as the animation is complete.
		 *   On Windows, the callback is called immediately (resizing doesn't
		 *   animate).
		 *   The callback is passed the new, actual size of the window.
		 */
		setWindowSize: (size: ISize, callback?: (newSize: ISize) => void) => void;

		/**
		 * Sets the minimum window size. The user won't be able to resize the
		 * app's window below the specified size.
		 *
		 * @param size
		 *   The minimum window size.
		 */
		setMinimumWindowSize: (size: ISize) => void;

		/**
		 * Displays a OS notification with a title and a message.
		 * On Mac, the notification will appear at the top right corner of the
		 * screen, on Windows it will appear at the taskbar notification area.
		 *
		 * @param title
		 *   The title of the notification.
		 *
		 * @param details
		 *   The message of the notification.
		 */
		displayNotification: (title: string, details: string) => void;

		/**
		 * Generates a visual cue to request the user's attention.
		 * On Mac, the icon in the dock will bounce; on Windows the icon in the
		 * task bar will flash.
		 */
        requestUserAttention: () => void;


		///////////////////////////////////////////////////////////////////////
		// Image Processing

		/**
		 * Converts any base64-encoded image data to a base64-encoded PNG to
		 * convert any image format known to Mac to a PNG (displayable in the
		 * web content of the app).
		 *
		 * Available only on Mac OS X.
		 *
		 * @param base64ImageData
		 *   The image as a base64-encoded string to convert to the PNG format.
		 *
		 * @param callback
		 *   Callback called with the data URI of the converted image.
		 */
		convertImage: (base64ImageData: string, callback: (base64PNG: string) => void) => void;

		/**
		 * Retrieves an image (screenshot) for the webpage at the URL "url"
		 * and returns the image as a base64-encoded PNG.
		 *
		 * @param url
		 *   The URL which to load and to take a screenshot of.
		 *
		 * @param width
		 *   The width of the image. The height will be adjusted automatically
		 *   to create an image with a 4:3 aspect ratio.
		 *
		 * @param callback
		 *   Callback called with a data URI of the web page's image.
		 */
		getPageImageForURL: (url: string, width: number, callback: (imageData: string) => void) => void;


		///////////////////////////////////////////////////////////////////////
		// Licensing

		/**
		 * Retrieves an ILicenseData object with information about the license
		 * used for the activation of the app.
		 * If the app was configured not to use licensing, null is returned.
		 * If the app wasn't activated yet by the user, all the fields of
		 * "data" passed to the callback will be empty strings.
		 *
		 * @param callback
		 *   Callback invoked with the license data or null if no licensing is
		 *   used in the app.
		 */
		getLicenseData: (callback: (data: ILicenseData) => void) => void;

		/**
		 * Revokes the app activation by removing the licensing information on
		 * the computer running the app.
		 */
		deactivateLicense: () => void;
	}


	export interface IPath
	{
		path: string;
        isDirectory?: boolean;

		/**
		 * OSX: file URL with bookmark
		 */
		url: string;

		hasSecurityAccessData: boolean;
	}

	export interface IFileChange
	{
		path: string;
		action: EFileChangeAction;
	}

	export enum EFileChangeAction
	{
		FILE_MODIFIED = 0,
		FILE_DELETED = 1
	}

	export interface IMenuItemFlags
	{
		[menuCommandId: string]: number;
	}

	export interface IMenuItem
	{
		caption: string;
		key?: string;
		
		/**
		 * Key modifier; bitwise combination of
		 * 1: Shift
		 * 2: Ctrl (Windows) / Cmd (Mac)
		 * 4: Alt
		 * 8: Ctrl (Mac)
		 */
		keyModifiers?: number;

		image?: string;
		menuCommandId?: string;
		systemCommandId?: string|number;
		subMenuItems?: IMenuItem[];
	}

	export interface IUpdaterSettings
	{
		autoCheck?: boolean;

		/**
		 * check frequency in seconds
		 */
		frequency?: number;

		/**
		 * Download the new version automatically.
		 * If this option isn't present, don't display it in the GUI.
		 */
		autoDownload?: boolean;
	}

	export interface IBrowser
	{
		name: string;
		version: string;
		id: string;

		/**
		 * Base64-encoded PNG of the browser icon
		 */
		image: string;

		isDefaultBrowser: boolean;
	}

	export interface IUserAgent
	{
		name: string;
		version: string;
		id: string;
		raw: string;
	}

	export interface IReadFileOptions
	{
		/**
		 * "encoding" can be one of
		 * - "utf-8"
		 * - "text/plain;utf-8"
		 * - "image/png;base64".
		 *
		 * If not provided, the file is read as plain text UTF-8 file.
		 * If "image/png;base64" is used as encoding, the file is interpreted
		 * as image file, converted to PNG, and returned as base-64 encoded
		 * data URL.
		 */
		encoding?: string;
	}

	export interface IAJAXOptions
	{
		type: string;
		url: string;
		dataType: string;
		success: (data: any, contentType: string) => void;
		error: (data: any) => void;
	}

	export interface IProxyConfig
	{
		/**
		 * The proxy type, one of
		 * - "none"
		 * - "http"
		 * - "https"
		 * - "socks"
		 */
		type: string;

		/**
		 * The proxy host.
		 */
		host?: string;

		/**
		 * The proxy port.
		 */
		port?: number;

		/**
		 * If the proxy requires authentication: the user name.
		 */
		username?: string;

		/**
		 * If the proxy requires authentication: the password.
		 */
		password?: string;
	}

	export interface ISize
	{
		width?: number;
		height?: number;
	}

	export enum EOutputStreamType
	{
		STDOUT = 0,
		STDERR = 2
	}

	export interface IOutputStreamData
	{
		fd: EOutputStreamType;
		text: string;
	}

	export interface ILicenseData
	{
		licenseKey: string;
		fullName: string;
		company: string;
	}
}

declare var app: NativeInterface.INative;