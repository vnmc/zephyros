Resources in static library


http://stackoverflow.com/questions/531502/vc-resources-in-a-static-library


"The only thing you need to do to use resources (images, dialogs, etc...) in a static library in Visual C++ (2008), is include the static library's associated .res file in your project. This can be done at "Project settings/Linker/Input/Additional dependencies".

With this solution, the resources of the static library are packed into the .exe, so you don't need an extra DLL. Regrettably, Visual Studio does not include the .res file automatically as it does for the .lib file (when using the "project dependencies"-feature), but I think this small extra step is acceptable.

I have looked for a very long time for this solution, and now it surprises me it is that simple. The only problem is that it is totally undocumented."