#!/usr/bin/env python2.7
import sys, random
from PyQt4 import QtGui, QtCore

class BBox:
    def __init__(self,minx,miny,maxx,maxy):
        self.minx = minx
        self.miny = miny
        self.maxx = maxx
        self.maxy = maxy

    def drawBox(self, qp):
        qp.drawRect( self.minx, self.miny, self.maxx-self.minx, self.maxy-self.miny )

class Visualization(QtGui.QWidget):
    def __init__(self,argv):
        super(Visualization,self).__init__()

        self.setGeometry(10,10,600,600)
        self.setWindowTitle('Segmentation Visualization')

        self.disparity_h_image = []
        self.disparity_v_image = []
        self.offset = []
        self.boxes = []
        self.globalOffset = QtCore.QPoint(0,0)
        self.globalStart = QtCore.QPoint(0,0)
        self.globalLastOffset = QtCore.QPoint(0,0)

        self.disparity_h_image.append(QtGui.QImage( argv[1][:-4]+"-H.tif"))
        self.disparity_v_image.append(QtGui.QImage( argv[1][:-4]+"-V.tif"))
        self.offset.append( QtCore.QPoint(0,0) )

        f = open("ZONE"+argv[1][-5:-4]+".txt", 'r')
        for line in f:
            chunks = line.split('Vector2')
            split_pt = chunks[1].find(',')
            minx = int(chunks[1][1:split_pt])
            miny = int(chunks[1][split_pt+1:-2])
            split_pt = chunks[2].find(',')
            maxx = int(chunks[2][1:split_pt])
            maxy = int(chunks[2][split_pt+1:-4])
            self.boxes.append( BBox(minx,miny,maxx,maxy) )

        # for idx in range(1,len(argv)):
        #     name = argv[idx]
        #     self.disparity_h_image.append(QtGui.QImage( name[:-4]+"-H.tif"))
        #     self.disparity_v_image.append(QtGui.QImage( name[:-4]+"-V.tif"))
        #     token = name.split("_")
        #     self.offset.append( QtCore.QPoint(int(token[1]),int(token[2])) )

        #     # Load up boxes
        #     f = open(name[:-4]+"_zone.txt",'r')
        #     for line in f:
        #         chunks = line.split('Vector2')
        #         split_pt = chunks[1].find(',')
        #         minx = int(chunks[1][1:split_pt]) + self.offset[-1].x()
        #         miny = int(chunks[1][split_pt+1:-2]) + self.offset[-1].y()
        #         split_pt = chunks[2].find(',')
        #         maxx = int(chunks[2][1:split_pt]) + self.offset[-1].x()
        #         maxy = int(chunks[2][split_pt+1:-4]) + self.offset[-1].y()
        #         self.boxes.append( BBox(minx,miny,maxx,maxy) )

        self.draw_other = False

    def paintEvent(self, e):
        qp = QtGui.QPainter()
        qp.begin(self)
        qp.translate(self.globalOffset)
        self.drawImage(qp)
        qp.setPen(QtGui.QColor('red'))
        for b in self.boxes:
            b.drawBox(qp)
        qp.end()

    def keyPressEvent(self, e):
        if e.key() == QtCore.Qt.Key_Space:
            self.draw_other = not self.draw_other
            self.update()

    def mouseDoubleClickEvent(self, e):
        # This should bring up a tooltip about the search region size
        pass

    def mousePressEvent(self, e):
        self.globalStart = e.pos()

    def mouseMoveEvent(self, e):
        self.globalOffset = e.pos() - self.globalStart + self.globalLastOffset
        self.update()

    def mouseReleaseEvent(self, e):
        self.globalLastOffset += e.pos() - self.globalStart

    def drawImage(self, qp):
        if self.draw_other:
            for idx in range(len(self.disparity_h_image)):
                qp.drawImage( self.offset[idx], self.disparity_v_image[idx] )
        else:
            for idx in range(len(self.disparity_v_image)):
                qp.drawImage( self.offset[idx], self.disparity_h_image[idx] )

app = QtGui.QApplication(sys.argv)
window = Visualization(sys.argv)
window.show()
app.exec_()
