#!/usr/bin/env python3
import xml.sax
from xml.dom import minidom
import xml.etree.ElementTree as xmltree
from xml.dom.minidom import parse, parseString
import os
import sys
import time
import tempfile
import filecmp
import shutil

class CDROIDHandler( xml.sax.ContentHandler ):
    def __init__(self,namespace):
        self.idlist=[]
        self.strings=[]
        self.namespace=namespace

    def normalizeXMLString(self,xmlStr):
        xmlStr.replace("\n", "&#10;")
        xmlStr.replace("\r", "&#0d;")
        return xmlStr

    def isMyNS(self,idname):#parse @android:id/ @+id/ @id/
        ns=""
        if idname.find(":")>0:
            ns = idname.split(":")
            ns =ns[0]
        else:
            pos = idname.find("id/")
            ns = idname[0:pos]
        ns= ns.replace("@","")
        ns= ns.replace("+","")
        #print(idname+"==>"+ns)
        if ns.strip():#not empty
            return (ns==namespace) or (ns=="android" and namespace=="cdroid") or (ns=="cdroid" and namespace=="android")
        return  True
    def startElement(self, tag, attributes):
        for attr in attributes.getNames():
            value = attributes.get(attr)
            if ':id' in attr:
                if self.isMyNS(value):
                    self.addID(value)
            if 'string/' in value:
                self.addString(value)
        if 'type' in attributes and 'name' in attributes:
            self.addID("@id/"+attributes.get('name'))

    def addID(self,name):
        pos=name.find('/')
        if pos<0:
            return
        name=name[pos+1:].strip()
        if (name[0].isalpha() or (name[0]=='_')) and (name not in self.idlist) :
            self.idlist.append(name)

    def addString(self,value):
        pos=value.find('/')
        if pos<0 :
            return
        #value=value[pos+1:].strip()
        if(value[0].isalpha() or (value[0]=='_')) and (value not in self.strings):
            self.strings.append(value)

    def groupby(self):
        new_ids = []
        for id in self.idlist:
            if id not in new_ids:
                new_ids.append(id)
        self.idlist=new_ids
        print(new_ids)
        return new_ids

class IDGenerater(object):
    def __init__(self,idstart,namespace):
        self.idstart=idstart
        self.namespace=namespace
        self.processDirs=['layout','drawable','navigation','values']
        self.parser = xml.sax.make_parser()
        self.parser.setFeature(xml.sax.handler.feature_namespaces, 0)
        self.Handler = CDROIDHandler(namespace)
        self.parser.setContentHandler( self.Handler )

    def dict2RH(self,filepath):
        fr=open(filepath,"w")
        fr.write("#pragma once\n\n")
        fr.write("/*Generated by machine ,Do not edit !!!*/\n\n")
        fr.write("namespace %s{\n\n"%(self.namespace))
        fr.write("namespace R{\n")
        fr.write("    namespace id{\n")
        i=0
        #print(self.Handler.idlist)
        #print(self.Handler.strings)
        for k in self.Handler.idlist:
            fr.write("%8s static constexpr int %-24s= 0x%08X ;/*%d*/\n"%('',k,self.idstart+i,self.idstart+i))
            i+=1 
        fr.write("%4s};/*namespace id*/\n\n"%(''))
        
        fr.write("    namespace strings{\n")
        i=0
        #for ss in self.Handler.strings:
        #    fr.write("%8s  static constexpr int %-24s= 0x%08X ;/*%d*/\n"%('',ss,self.idstart+i,self.idstart+i))
        #    i+=1
        fr.write("%4s};/*namespace strings*/\n\n"%(''))

        fr.write("};//endof namespace R\n\n")
        fr.write("}//endof namespace\n\n")
        fr.close()

    def dict2ID(self,filepath):
        fr=open(filepath,"w")
        fr.write('<?xml version="1.0" encoding="utf-8"?>')
        fr.write("<!--Generated by CDdroid's machine,Do not edit !!!-->\n")
        fr.write("<resources xmlns:android=\"http://schemas.android.com/apk/res/android\">\n")
        i=0;
        for k in self.Handler.idlist:
            fr.write('  <id name="%s">0x%08x</id>\n'%(k,self.idstart+i))
            i+=1
        fr.write("</resources>\n\n")
        fr.close();

    def elementExists(self,elements,tofind):
        for e in elements:
            if e.getAttribute('name')==tofind:
                return 1
        return 0

    def fixed_writexml(self,writer,xmlroot): #, indent, addindent, newl,encoding):
        indent=''
        addindent='\t'
        newl='\n'
        encoding='utf-8'
        writer.write(indent + "<" + xmlroot.tagName)

        attrs = xmlroot._get_attributes()
        a_names = attrs.keys()

        for a_name in a_names:
            writer.write(" %s=\"" % a_name)
            minidom._write_data(writer, attrs[a_name].value)
            writer.write("\"")
        if xmlroot.childNodes:
            if len(xmlroot.childNodes) == 1 \
                and xmlroot.childNodes[0].nodeType == minidom.Node.TEXT_NODE:
                writer.write(">")
                xmlroot.childNodes[0].writexml(writer, "", "", "")
                writer.write("</%s>%s" % (xmlroot.tagName, newl))
                return
            writer.write(">%s" % (newl))
            for node in xmlroot.childNodes:
                if node.nodeType is not minidom.Node.TEXT_NODE:
                    node.writexml(writer, indent + addindent, addindent, newl)
            writer.write("%s</%s>%s" % (indent, xmlroot.tagName, newl))
        else:
            writer.write("/>%s" % (newl))

    def strings2XML(self,filename):
        print(self.Handler.strings)
        try:
            dom=parse(filename)
        except:#(xml.parsers.expat.ExpatError)
            print(filename+" open failed or syntax error")
            dom=parseString('<?xml version="1.0" encoding="utf-8"?>\n<resources lang="zh_CN"></resources>')

        root = dom.documentElement
        stringsNodeInFile=root.getElementsByTagName('string')
        for str in self.Handler.strings:
            if self.elementExists(stringsNodeInFile,str):
                continue
            e=dom.createElement('string')
            text = dom.createTextNode(str)
            text.text=str
            e.setAttribute('name',str)
            e.appendChild(text)
            root.appendChild(e)
        fp = open(filename, mode='w')#, encoding='utf-8')
        self.fixed_writexml(fp,root)# ,indent='', addindent='\t', newl='\n', encoding='utf-8')
        fp.close()

    def dirHasId(self,path):
        for dir in self.processDirs:
            if path.find(dir)>=0:
                return 1
        return -1;

    def scanxml(self,scanPath):
        lastmodifytime=0
        for top, dirs, nondirs in os.walk(scanPath):
            dirs.sort()
            nondirs.sort()
            for item in nondirs:
                fname=os.path.join(top, item)
                if (not item.endswith('.xml')) or (self.dirHasId(fname)<0):
                    continue
                newestdate=os.stat(fname);
                if(lastmodifytime<newestdate.st_mtime):
                    lastmodifytime=newestdate.st_mtime
                self.parser.parse(fname)
        return lastmodifytime

if ( __name__ == "__main__"):
    #arg0 is myname(idgen.py)
    #arg1 is namespace
    #arg2 is resource directory
    #arg3 is directory which R.h generate to
    idstart=10000
    if len(sys.argv)<3:
        print(sys.argv[0])
    print(sys.argv)
    namespace=sys.argv[1]
    if namespace=='cdroid':
        idstart=1000
    print("namespace="+namespace)
    lastmodifytime=0 
    if os.path.exists(sys.argv[3]):
        fstat=os.stat(sys.argv[3])
        lastmodifytime=fstat.st_mtime
    idgen=IDGenerater(idstart,namespace)
    if not os.path.exists(sys.argv[2]+"/values"):
        os.makedirs(sys.argv[2]+"/values")

    idgen.scanxml(sys.argv[2])
    fd,ftempids=tempfile.mkstemp(prefix=namespace+"_ID", suffix=".xml")
    idgen.dict2ID(ftempids)
    fidxml = sys.argv[2]+"/values/ID.xml"
    isIDSame = os.path.exists(fidxml) and filecmp.cmp(ftempids,fidxml)
    msg = "not changed "
    if not isIDSame or not os.path.exists(sys.argv[3]):#True if same,otherwise False
        #content is changed,we must copy ftempids to fidxml(sys.argv[2]+"/values/ID.xml)
        shutil.copyfile(ftempids,fidxml)
        idgen.dict2RH(sys.argv[3])
        msg="changed "
    print(namespace+"'s IDs is "+msg+ftempids+" : "+fidxml)
    try:
        os.remove(ftempids)
    except OSError as e:
        print("Remove File:{e}")
