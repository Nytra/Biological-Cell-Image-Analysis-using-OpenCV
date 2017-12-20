from ij import IJ
from ij.plugin import ChannelSplitter
from ij.plugin.filter import RankFilters, MaximumFinder
from ij.gui import GenericDialog
from ij.measure import ResultsTable
import os

# ToDo: Overlay points onto the other channels and find intensities.
# 		Plot graph of results.

class LSM:

	def __init__(self, channels):
		self.channels = channels
		self.points = []

	def findPoints(self):
		imgProc = self.channels[0].getProcessor() # ByteProcessor()
		colorProc = imgProc.convertToColorProcessor()
		RankFilters().rank(imgProc, 2, RankFilters.MEDIAN) # rank(imgProc, radius, filter)
		
		try:
			textPanel = ResultsTable().getResultsWindow().getTextPanel()
			textPanel.clear()
			flag = False
		except AttributeError:
			flag = True
			
		MaximumFinder().findMaxima(imgProc, 75, MaximumFinder.LIST, 0) # findMaxima(imgProc, tolerance, outputType, excludeOnEdges) 

		if flag:
			textPanel = ResultsTable().getResultsWindow().getTextPanel()
		
		for i in range(textPanel.getLineCount()):
			line = textPanel.getLine(i).strip()
			n, x, y = line.split("\t")
			self.points.append([int(x), int(y)])

	def getPoints(self):
		return self.points


path = IJ.getDirectory("Select Source Directory")

data = []
filenames = list(name for name in os.listdir(path) if name.endswith(".lsm"))
for i, filename in enumerate(filenames):
	imgPlus = IJ.openImage(path + "/" + filename)
	channels = ChannelSplitter.split(imgPlus)
	data.append(LSM(channels))

print("LSM Files Loaded:", len(data))

for lsm in data:
	lsm.findPoints()
















