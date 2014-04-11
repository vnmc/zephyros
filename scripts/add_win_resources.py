# Add resource files to Windows version

import os, sys


base_id = 3000
res_file_extensions = [ '.html', '.css', '.js', '.png', '.woff', '.ttf', '.svg', '.jpg', '.jpeg' ]
exclude_files = [ 'app\\js\\mock.app.js' ]


# ------------------------------------------------------------------------------
# Read resource files

resfiles = []
for dirname, dirnames, filenames in os.walk('..\\app'):
	# print path to all filenames.
	for filename in filenames:
		for ext in res_file_extensions:
			if filename.endswith(ext):
				path = os.path.join(dirname, filename).replace('..\\app\\', '')
				if not (path in exclude_files):
					resfiles.append(path)
					print(path)


# ------------------------------------------------------------------------------
# Add resources to the RC file

# read file
rcfile = open('../src/app.rc', 'r')
rclines = []
adding_binary = False
binaries_added = False
icon_include_pos = -1
linenum = 0

for line in rcfile:
	# start adding lines after this marker again
	if line.find('/////////////////////////////////////////////////////////////////////////////') >= 0:
		adding_binary = False
	if line.find('// Icon') == 0:
		icon_include_pos = linenum

	if not adding_binary:
		rclines.append(line)

	# add the resources; skip lines until marker
	if (line.find('// Binary') == 0) or (line.find('// 256') == 0):
		adding_binary = True

		rclines.append('//\n\n')
		i = 1
		for resource in resfiles:
			rclines.append(str(base_id + i) + ' BINARY "..\\\\app\\\\' + resource.replace('\\', '\\\\') + '"\n')
			i += 1
		rclines.append('\n')
		rclines.append('\n')
		binaries_added = True

	linenum += 1

if not binaries_added:
	print('binaries not found in existing .rc file, adding at position %d' % icon_include_pos)
	rclines.insert(icon_include_pos - 5, '/////////////////////////////////////////////////////////////////////////////\n')
	rclines.insert(icon_include_pos - 4, '//\n')
	rclines.insert(icon_include_pos - 3, '// Binary\n')
	rclines.insert(icon_include_pos - 2, '//\n\n')

	i = 1
	for resource in resfiles:
		rclines.insert(icon_include_pos + i - 2, str(base_id + i) + ' BINARY "..\\\\app\\\\' + resource.replace('\\', '\\\\') + '"\n')
		i += 1
	rclines.insert(icon_include_pos + i - 2, '\n')
	rclines.insert(icon_include_pos + i - 1, '\n')


# write it back
rcfile.close()
rcfile = open('../src/app.rc', 'w')
rcfile.writelines(rclines)
rcfile.close()
#sys.stdout.writelines(rclines)


# ------------------------------------------------------------------------------
# Add resources to the map in the CPP file

# read file
cppfile = open('../src/resource_util_win.cpp', 'r')
cpplines = []
adding_resources = False

for line in cppfile:
	# start adding lines after this marker again
	if line.find('@RESOURCE_MAPPING_END') >= 0:
		adding_resources = False

	if not adding_resources:
		cpplines.append(line)

	# add the resources; skip lines until marker
	if line.find('@RESOURCE_MAPPING_START') >= 0:
		adding_resources = True

		i = 1
		for resource in resfiles:
			cpplines.append('\t\t{ TEXT("' + resource.replace('\\', '/') + '"), ' + str(base_id + i) + ' },\n')
			i += 1


# write it back
cppfile = open('../src/resource_util_win.cpp', 'w')
cppfile.writelines(cpplines)
cppfile.close()
#sys.stdout.writelines(cpplines)