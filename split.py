from ij import IJ
from ij.gui import GenericDialog
from ij.plugin import ChannelSplitter
import os

path = IJ.getDirectory("Select source directory:")
	
if not os.path.exists(path + "\\output"):
	os.mkdir(path + "\\output")

filenames = list(name for name in os.listdir(path) if name.endswith(".lsm"))
for filename in filenames:
	img = IJ.openImage(path + "\\" + filename)
	#IJ.saveAsTiff(img, path + "\\output\\" + filename[:-4])
	channels = ChannelSplitter.split(img)
	for i, c in enumerate(channels):
		IJ.saveAsTiff(c, path + "\\output\\" + filename[:-4] + "_channel" + str(i))

IJ.beep()
gd = GenericDialog("Success!")
gd.addMessage("Successfully split " + str(len(filenames)) + " hyperstacks.")
gd.showDialog()