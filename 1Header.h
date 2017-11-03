< ? xml version = "1.0" ? >
<vstgui - ui - description version = "1">
<fonts>
<font bold = "true" font - name = "Microsoft Sans Serif" name = "Microsoft Sans Serif" size = "18" / >
< / fonts>
<colors>
<color name = "kLightGreyCColor" rgba = "#c8c8c8ff" / >
<color name = "kDarkGreyCColor" rgba = "#646464ff" / >
<color name = "Orange" rgba = "#FF952Dff" / >
<color name = "Purple" rgba = "#8100CDff" / >
< / colors>
<template background - color = "~ WhiteCColor" background - color - draw - style = "filled and stroked" bitmap = "lord" class = "CViewContainer" maxSize = "1000, 1000" minSize = "1000, 1000" mouse - enabled = "true" name = "Editor" origin = "0, 0" size = "1000, 1000" transparent = "false">
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "145 ,370" rafxtemplate - type = "knobgroup" size = "91 ,107" template = "Rafx KnobGroup 2" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "130 ,260" rafxtemplate - type = "knobgroup" size = "113 ,99" template = "Rafx KnobGroup 4" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "115 ,565" rafxtemplate - type = "slidergroup" size = "158 ,297" template = "Rafx SliderGroup 1" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "140 ,160" rafxtemplate - type = "slidergroup" size = "109 ,95" template = "Rafx SliderGroup 0" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "135 ,470" rafxtemplate - type = "knobgroup" size = "101 ,95" template = "Rafx KnobGroup 1" transparent = "true" / >
<view background - offset = "0, 0" bitmap = "hsbackS700" bitmap - offset = "0, 0" class = "CSlider" control - tag = "Dry/Wet" custom - view - name = "" default - value = "0" draw - back = "false" draw - back - color = "~ WhiteCColor" draw - frame = "false" draw - frame - color = "~ WhiteCColor" draw - value = "false" draw - value - color = "~ WhiteCColor" draw - value - from - center = "false" draw - value - inverted = "false" handle - bitmap = "hshandle" handle - offset = "0, 1" max - value = "1" min - value = "0" mode = "free click" mouse - enabled = "true" orientation = "horizontal" origin = "140 ,890" reverse - orientation = "false" size = "96 ,32" sub - controller = "" transparent = "false" transparent - handle = "true" wheel - inc - value = "0.1" zoom - factor = "10" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "UnitsLabel" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ RedCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "125 ,860" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "131 ,16" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "center" text - inset = "0, 0" title = "Dry/Wet Reverb Mix" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Dry/Wet" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "150 ,930" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,13" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "UnitsLabel" default - value = "0.5" font = "~ NormalFontVeryBig" font - antialias = "true" font - color = "~ BlackCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "190 ,5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "510 ,50" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "center" text - inset = "0, 0" title = "Cast Multi-FX" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view background - offset = "0, 0" bitmap = "twossamber" class = "COnOffButton" control - tag = "Reverb" default - value = "0" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "135 ,100" size = "120 ,42" transparent = "false" wheel - inc - value = "0.1" height - of - one - image = "20" custom - view - name = "" / >
<view background - offset = "0, 0" bitmap = "twossamber" class = "COnOffButton" control - tag = "Delay" default - value = "0" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "405 ,100" size = "120 ,42" transparent = "false" wheel - inc - value = "0.1" height - of - one - image = "20" custom - view - name = "" / >
<view background - offset = "0, 0" bitmap = "twossamber" class = "COnOffButton" control - tag = "Bitcrusher" default - value = "0" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "705 ,105" size = "120 ,42" transparent = "false" wheel - inc - value = "0.1" height - of - one - image = "20" custom - view - name = "" / >
<view background - offset = "0, 0" bitmap = "bigpowerswitch" class = "COnOffButton" control - tag = "Bypass" default - value = "0" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "20 ,25" size = "55 ,47" transparent = "false" wheel - inc - value = "0.1" height - of - one - image = "52" custom - view - name = "" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "420 ,320" rafxtemplate - type = "knobgroup" size = "75 ,85" template = "Rafx KnobGroup 0" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "420 ,430" rafxtemplate - type = "knobgroup" size = "78 ,85" template = "Rafx KnobGroup 3" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "420 ,540" rafxtemplate - type = "knobgroup" size = "81 ,85" template = "Rafx KnobGroup 5" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "420 ,640" rafxtemplate - type = "knobgroup" size = "85, 80" template = "Rafx KnobGroup 6" transparent = "true" sub - controller = "" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "COptionMenu" control - tag = "Mod Type" custom - view - name = "" default - value = "0.5" font = "~ NormalFont" font - antialias = "true" font - color = "~ GreenCColor" frame - color = "~ WhiteCColor" frame - width = "1" max - value = "" menu - check - style = "false" menu - popup - style = "false" min - value = "0" mouse - enabled = "true" origin = "415 ,180" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "92 ,27" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "SEL 1" transparent = "true" value - precision = "0" wheel - inc - value = "0.1" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "COptionMenu" control - tag = "LFO" custom - view - name = "" default - value = "0.5" font = "~ NormalFont" font - antialias = "true" font - color = "~ RedCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "" menu - check - style = "false" menu - popup - style = "false" min - value = "0" mouse - enabled = "true" origin = "415 ,235" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "92 ,27" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "SEL 1" transparent = "true" value - precision = "0" wheel - inc - value = "0.1" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "COptionMenu" control - tag = "Phase" custom - view - name = "" default - value = "0.5" font = "~ NormalFont" font - antialias = "true" font - color = "~ YellowCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "" menu - check - style = "false" menu - popup - style = "false" min - value = "0" mouse - enabled = "true" origin = "415 ,285" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "92 ,27" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "SEL 1" transparent = "true" value - precision = "0" wheel - inc - value = "0.1" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "720 ,170" rafxtemplate - type = "knobgroup" size = "75 ,85" template = "Rafx KnobGroup 8" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "720 ,270" rafxtemplate - type = "knobgroup" size = "87 ,94" template = "Rafx KnobGroup 9" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "720 ,370" rafxtemplate - type = "knobgroup" size = "91 ,101" template = "Rafx KnobGroup 10" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "720 ,490" rafxtemplate - type = "knobgroup" size = "75 ,85" template = "Rafx KnobGroup 11" transparent = "true" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "720 ,600" rafxtemplate - type = "knobgroup" size = "75 ,85" template = "Rafx KnobGroup 12" transparent = "true" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "UnitsLabel" default - value = "0.5" font = "~ NormalFontBig" font - antialias = "true" font - color = "Purple" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "145 ,75" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "100 ,16" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "center" text - inset = "0, 0" title = "Reverb" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "UnitsLabel" default - value = "0.5" font = "~ NormalFontBig" font - antialias = "true" font - color = "Purple" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "415 ,75" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "100 ,16" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "center" text - inset = "0, 0" title = "Delay" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "UnitsLabel" default - value = "0.5" font = "~ NormalFontBig" font - antialias = "true" font - color = "Purple" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "710 ,75" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "100 ,16" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "center" text - inset = "0, 0" title = "BitCrusher" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "UnitsLabel" default - value = "0.5" font = "~ NormalFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "-5 ,5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "100 ,16" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "center" text - inset = "0, 0" title = "ByPass" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" origin = "420 ,740" rafxtemplate - type = "knobgroup" size = "75 ,85" template = "Rafx KnobGroup 7" transparent = "true" / >
< / template>
<custom>
<attributes RAFXPath = "C:\Users\Joepetey\Desktop\MMI Projects\TappedReverb\RackAFX.uidesc" name = "VST3Editor" / >
<attributes name = "FocusDrawing" / >
<attributes KnobAction = "2" / >
<attributes name = "UIGridController" / >
<attributes SelectedTemplate = "Editor" name = "UITemplateController" / >
<attributes name = "UIAttributesController" / >
<attributes EditorSize = "0, 0, 1236, 768" SplitViewSize_0_0 = "0.693135935397038990000" SplitViewSize_0_1 = "0.293405114401076690000" SplitViewSize_1_0 = "0.479138627187079390000" SplitViewSize_1_1 = "0.50740242261103630000" SplitViewSize_2_0 = "0.63187702265372170000" SplitViewSize_2_1 = "0.360032362459546950000" TabSwitchValue = "1" name = "UIEditController" / >
<attributes SelectedRow = "-1" name = "UIViewCreatorDataSource" / >
<attributes SelectedRow = "-1" name = "UITagsDataSource" / >
<attributes AppendUnits = "true" / >
<attributes TransparentEdits = "true" / >
<attributes default - blurb - color = "~ BlackCColor" / >
<attributes RAFXShowGrid = "false" / >
<attributes RAFXAutosizeKnobs = "false" / >
<attributes RAFXAutosize = "false" / >
<attributes RAFXGridSize = "10" / >
<attributes RAFXCanvasWidth = "1000" / >
<attributes RAFXCanvasHeight = "1000" / >
<attributes rafx - template - name = "Rafx KnobGroup 2" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 4" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx SliderGroup 1" rafxtemplate - type = "slidergroup" / >
<attributes rafx - template - name = "Rafx SliderGroup 0" rafxtemplate - type = "slidergroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 1" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 0" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 3" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 5" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 6" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 8" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 9" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 10" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 11" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 12" rafxtemplate - type = "knobgroup" / >
<attributes rafx - template - name = "Rafx KnobGroup 7" rafxtemplate - type = "knobgroup" / >
< / custom>
<bitmaps>
<bitmap name = "darkgreyvinyl" nineparttiled - offsets = "0, 0, 0, 0" path = "darkgreyvinyl.png" rafxtype = "backgnd" / >
<bitmap name = "bluemetal" nineparttiled - offsets = "0, 0, 0, 0" path = "bluemetal.png" rafxtype = "backgnd" / >
<bitmap name = "greenmetal" nineparttiled - offsets = "0, 0, 0, 0" path = "greenmetal.png" rafxtype = "backgnd" / >
<bitmap name = "greymetal" nineparttiled - offsets = "0, 0, 0, 0" path = "greymetal.png" rafxtype = "backgnd" / >
<bitmap name = "lightgreymetal" nineparttiled - offsets = "0, 0, 0, 0" path = "lightgreymetal.png" rafxtype = "backgnd" / >
<bitmap name = "yellowmetal" nineparttiled - offsets = "0, 0, 0, 0" path = "yellowmetal.png" rafxtype = "backgnd" / >
<bitmap name = "background" nineparttiled - offsets = "150, 160, 150, 160" path = "background.png" rafxtype = "backgnd" / >
<bitmap name = "groupframe" nineparttiled - offsets = "25, 25, 25, 25" path = "groupframe.png" rafxtype = "groupframe" / >
<bitmap name = "knob" path = "knob.png" rafxtype = "knob" / >
<bitmap name = "blackgearknob" path = "blackgearknob.png" rafxtype = "knob" / >
<bitmap name = "blueableton" path = "blueableton.png" rafxtype = "knob" / >
<bitmap name = "chickenhead" path = "chickenhead.png" rafxtype = "knob" / >
<bitmap name = "fxknob" path = "fxknob.png" rafxtype = "knob" / >
<bitmap name = "greenableton" path = "greenableton.png" rafxtype = "knob" / >
<bitmap name = "greyableton" path = "greyableton.png" rafxtype = "knob" / >
<bitmap name = "knob303" path = "knob303.png" rafxtype = "knob" / >
<bitmap name = "knobm400" path = "knobm400.png" rafxtype = "knob" / >
<bitmap name = "minibrute" path = "minibrute.png" rafxtype = "knob" / >
<bitmap name = "sslblue" path = "sslblue.png" rafxtype = "knob" / >
<bitmap name = "sslgreen" path = "sslgreen.png" rafxtype = "knob" / >
<bitmap name = "sslred" path = "sslred.png" rafxtype = "knob" / >
<bitmap name = "alphablackgearknob1" path = "alphablackgearknob1.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphablackgearknob2" path = "alphablackgearknob2.png" rafxtype = "userimage" / >
<bitmap name = "alphachickenhead" path = "alphachickenhead.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphafxknob" path = "alphafxknob.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphagreenableton" path = "alphagreenableton.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphaknob303" path = "alphaknob303.png" rafxtype = "alphaknob270" / >
<bitmap name = "alpham400" path = "alpham400.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphaminibrute" path = "alphaminibrute.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphasslblue" path = "alphasslblue.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphasslgreen" path = "alphasslgreen.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphasslred" path = "alphasslred.png" rafxtype = "alphaknob270" / >
<bitmap name = "amberonoff" path = "amberonoff.png" rafxtype = "onoffbtn" / >
<bitmap name = "twossamber" path = "twossamber.png" rafxtype = "onoffbtn" / >
<bitmap name = "twossgreen" path = "twossgreen.png" rafxtype = "onoffbtn" / >
<bitmap name = "assignbamber" path = "assignbamber.png" rafxtype = "assignbtn" / >
<bitmap name = "rbstripamber2" path = "rbstripamber2.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripamber3" path = "rbstripamber3.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripamber4" path = "rbstripamber4.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripamber5" path = "rbstripamber5.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripamber6" path = "rbstripamber6.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripamber7" path = "rbstripamber7.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripamber8" path = "rbstripamber8.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripgreen2" path = "rbstripgreen2.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripgreen3" path = "rbstripgreen3.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripgreen4" path = "rbstripgreen4.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripgreen5" path = "rbstripgreen5.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripgreen6" path = "rbstripgreen6.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripgreen7" path = "rbstripgreen7.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripgreen8" path = "rbstripgreen8.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripsilverbutton2" path = "rbstripsilverbutton2.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripsilverbutton3" path = "rbstripsilverbutton3.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripsilverbutton4" path = "rbstripsilverbutton4.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripsilverbutton5" path = "rbstripsilverbutton5.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripsilverbutton6" path = "rbstripsilverbutton6.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripsilverbutton7" path = "rbstripsilverbutton7.png" rafxtype = "radiobtn" / >
<bitmap name = "rbstripsilverbutton8" path = "rbstripsilverbutton8.png" rafxtype = "radiobtn" / >
<bitmap name = "vsbackground" path = "vsbackground.png" rafxtype = "vsliderback" / >
<bitmap name = "hsbackground" path = "hsbackground.png" rafxtype = "hsliderback" / >
<bitmap name = "hsbackS700" path = "hsbackS700.png" rafxtype = "hsliderback" / >
<bitmap name = "vsbackgroundtwo" path = "vsbackgroundtwo.png" rafxtype = "vsliderback" / >
<bitmap name = "vshandle" path = "vshandle.png" rafxtype = "vsliderhand" / >
<bitmap name = "hshandS700" path = "hshandS700.png" rafxtype = "hsliderhand" / >
<bitmap name = "hshandle" path = "hshandle.png" rafxtype = "hsliderhand" / >
<bitmap name = "vuledoff" path = "vuledoff.png" rafxtype = "vuoff" / >
<bitmap name = "vuledon" path = "vuledon.png" rafxtype = "vuon" / >
<bitmap name = "vuledoffinv" path = "vuledoffinv.png" rafxtype = "vuoffinv" / >
<bitmap name = "vuledoninv" path = "vuledoninv.png" rafxtype = "vuoninv" / >
<bitmap name = "vsbackshort56" path = "vsbackshort56.png" rafxtype = "vsliderback" / >
<bitmap name = "vshandsmall56" path = "vshandsmall56.png" rafxtype = "vsliderhand" / >
<bitmap name = "hsbackshort56" path = "hsbackshort56.png" rafxtype = "hsliderback" / >
<bitmap name = "hshandsmall56" path = "hshandsmall56.png" rafxtype = "hsliderhand" / >
<bitmap name = "alphaampknob" path = "alphaampknob.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphachickenhead2" path = "alphachickenhead2.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphachickenhead3" path = "alphachickenhead3.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphamaschine" path = "alphamaschine.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphaorangeamp" path = "alphaorangeamp.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphasslblue2" path = "alphasslblue2.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphasslgreen2" path = "alphasslgreen2.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphasslred2" path = "alphasslred2.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphasslyellow" path = "alphasslyellow.png" rafxtype = "alphaknob270" / >
<bitmap name = "alphawhitedust" path = "alphawhitedust.png" rafxtype = "alphaknob270" / >
<bitmap name = "ampknob" path = "ampknob.png" rafxtype = "knob" / >
<bitmap name = "bigampknob" path = "bigampknob.png" rafxtype = "otherknob" / >
<bitmap name = "bigoberheimbutton" path = "bigoberheimbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "bigpowerswitch" path = "bigpowerswitch.png" rafxtype = "onoffbtn" / >
<bitmap name = "bigprohphetbutton" path = "bigprohphetbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "bigrubberbutton" path = "bigrubberbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "bigtoggleswitch" path = "bigtoggleswitch.png" rafxtype = "onoffbtn" / >
<bitmap name = "chickenhead2" path = "chickenhead2.png" rafxtype = "knob" / >
<bitmap name = "chickenhead3" path = "chickenhead3.png" rafxtype = "knob" / >
<bitmap name = "giantampknob" path = "giantampknob.png" rafxtype = "otherknob" / >
<bitmap name = "maschine" path = "maschine.png" rafxtype = "knob" / >
<bitmap name = "medoberheimbutton" path = "medoberheimbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "medpowerswitch" path = "medpowerswitch.png" rafxtype = "onoffbtn" / >
<bitmap name = "medprophetbutton" path = "medprophetbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "medrubberbutton" path = "medrubberbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "medtoggleswitch" path = "medtoggleswitch.png" rafxtype = "onoffbtn" / >
<bitmap name = "microoberheimbutton" path = "microoberheimbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "microtoggleswitch" path = "microtoggleswitch.png" rafxtype = "onoffbtn" / >
<bitmap name = "orangeamp" path = "orangeamp.png" rafxtype = "knob" / >
<bitmap name = "smalloberheimbutton" path = "smalloberheimbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "smallpowerswitch" path = "smallpowerswitch.png" rafxtype = "onoffbtn" / >
<bitmap name = "smallprohetbutton" path = "smallprohetbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "smallrubberbutton" path = "smallrubberbutton.png" rafxtype = "onoffbtn" / >
<bitmap name = "smalltoggleswitch" path = "smalltoggleswitch.png" rafxtype = "onoffbtn" / >
<bitmap name = "tinytoggleswitch" path = "tinytoggleswitch.png" rafxtype = "onoffbtn" / >
<bitmap name = "sslblue2" path = "sslblue2.png" rafxtype = "knob" / >
<bitmap name = "sslgreen2" path = "sslgreen2.png" rafxtype = "knob" / >
<bitmap name = "sslyellow" path = "sslyellow.png" rafxtype = "knob" / >
<bitmap name = "sslyellow2" path = "sslyellow2.png" rafxtype = "knob" / >
<bitmap name = "whitedust" path = "whitedust.png" rafxtype = "knob" / >
<bitmap name = "whitedust" path = "whitedust.png" rafxtype = "knob" / >
<bitmap name = "prophet" path = "prophet.png" rafxtype = "knob" / >
<bitmap name = "alphaprophet" path = "alphaprophet.png" rafxtype = "knob" / >
<bitmap name = "giantprophet" path = "giantprophet.png" rafxtype = "knob" / >
<bitmap name = "smallprophet" path = "smallprophet.png" rafxtype = "knob" / >
<bitmap name = "tinyprophet" path = "tinyprophet.png" rafxtype = "knob" / >
<bitmap name = "blackplastic" path = "blackplastic.png" rafxtype = "knob" / >
<bitmap name = "alphablackplastic" path = "alphablackplastic.png" rafxtype = "knob" / >
<bitmap name = "giantblackplastic" path = "giantblackplastic.png" rafxtype = "knob" / >
<bitmap name = "smallblackplastic" path = "smallblackplastic.png" rafxtype = "knob" / >
<bitmap name = "analogvumeter" path = "analogvumeter.png" rafxtype = "analogvu" / >
<bitmap name = "smallanalogvumeter" path = "smallanalogvumeter.png" rafxtype = "analogvu" / >
<bitmap name = "biganalogvumeter" path = "biganalogvumeter.png" rafxtype = "analogvu" / >
<bitmap name = "medanalogvumeter" path = "medanalogvumeter.png" rafxtype = "analogvu" / >
<bitmap name = "horizvuledoff" path = "horizvuledoff.png" rafxtype = "vuoff" / >
<bitmap name = "horizvuledoffinv" path = "horizvuledoffinv.png" rafxtype = "vuoffinv" / >
<bitmap name = "horizvuledon" path = "horizvuledon.png" rafxtype = "vuon" / >
<bitmap name = "horizvuledoninv" path = "horizvuledoninv.png" rafxtype = "vuoninv" / >
<bitmap name = "vertswitchfilters" path = "vertswitchfilters.png" rafxtype = "vertswitch" / >
<bitmap name = "horizswitchsynth" path = "horizswitchsynth.png" rafxtype = "horizswitch" / >
<bitmap name = "knobvst" path = "knobvst.png" / >
<bitmap name = "bigknob2" path = "bigknob2.png" / >
<bitmap name = "pc3envfader01orange" path = "pc3envfader01orange.png" / >
<bitmap name = "coolback" path = "coolback.png" / >
<bitmap name = "lord" path = "lord.png" rafxtype = "backgnd" / >
<bitmap name = "please" path = "please.png" / >
< / bitmaps>
<control - tags>
<control - tag name = "Pre Delay Reverb" tag = "0" / >
<control - tag name = "PD Atten" tag = "1" / >
<control - tag name = "Bndwidth" tag = "2" / >
<control - tag name = "APF1 Dly" tag = "3" / >
<control - tag name = "APF1 g" tag = "4" / >
<control - tag name = "APF2 Dly" tag = "5" / >
<control - tag name = "APF2 g" tag = "6" / >
<control - tag name = "KRT" tag = "7" / >
<control - tag name = "RT60" tag = "8" / >
<control - tag name = "Dry/Wet" tag = "9" / >
<control - tag name = "PComb1 Dly" tag = "10" / >
<control - tag name = "PComb2 Dly" tag = "11" / >
<control - tag name = "PComb3 Dly" tag = "12" / >
<control - tag name = "PComb4 Dly" tag = "13" / >
<control - tag name = "APF3 Dly" tag = "14" / >
<control - tag name = "APF3 g" tag = "15" / >
<control - tag name = "PComb5 Dly" tag = "16" / >
<control - tag name = "PComb6 Dly" tag = "17" / >
<control - tag name = "PComb7 Dly" tag = "18" / >
<control - tag name = "PComb8 Dly" tag = "19" / >
<control - tag name = "APF4 Dly" tag = "20" / >
<control - tag name = "APF4 g" tag = "21" / >
<control - tag name = "Rate" tag = "22" / >
<control - tag name = "Depth" tag = "23" / >
<control - tag name = "Resonance" tag = "24" / >
<control - tag name = "Pre Delay" tag = "25" / >
<control - tag name = "Chorus Offset" tag = "26" / >
<control - tag name = "APF5 Dly" tag = "27" / >
<control - tag name = "APF5 g" tag = "28" / >
<control - tag name = "Multi-Tap Delay" tag = "29" / >
<control - tag name = "APF6 Dly" tag = "30" / >
<control - tag name = "APF6 g" tag = "31" / >
<control - tag name = "HPCutoff" tag = "32" / >
<control - tag name = "APF7 Dly" tag = "33" / >
<control - tag name = "APF7 g" tag = "34" / >
<control - tag name = "APF8 Dly" tag = "35" / >
<control - tag name = "APF8 g" tag = "36" / >
<control - tag name = "Pitch Emulator" tag = "37" / >
<control - tag name = "Wet/Dry" tag = "38" / >
<control - tag name = "bitRate" tag = "39" / >
<control - tag name = "Downsampling Ratio" tag = "40" / >
<control - tag name = "Bit Depth" tag = "41" / >
<control - tag name = "Bitcrusher" tag = "42" / >
<control - tag name = "Reverb" tag = "43" / >
<control - tag name = "Delay" tag = "44" / >
<control - tag name = "Mod Type" tag = "45" / >
<control - tag name = "LFO" tag = "46" / >
<control - tag name = "Phase" tag = "47" / >
<control - tag name = "Bypass" tag = "48" / >
<control - tag name = "ASSIGNBUTTON_1" tag = "32768" / >
<control - tag name = "ASSIGNBUTTON_2" tag = "32769" / >
<control - tag name = "ASSIGNBUTTON_3" tag = "32770" / >
<control - tag name = "JOYSTICK_X" tag = "32773" / >
<control - tag name = "JOYSTICK_Y" tag = "32774" / >
<control - tag name = "JOYSTICK" tag = "32775" / >
<control - tag name = "TRACKPAD" tag = "32776" / >
<control - tag name = "TAB_CTRL_0" tag = "65536" / >
<control - tag name = "TAB_CTRL_1" tag = "65537" / >
<control - tag name = "TAB_CTRL_2" tag = "65538" / >
<control - tag name = "TAB_CTRL_3" tag = "65539" / >
<control - tag name = "TAB_CTRL_4" tag = "65540" / >
<control - tag name = "TAB_CTRL_5" tag = "65541" / >
<control - tag name = "TAB_CTRL_6" tag = "65542" / >
<control - tag name = "TAB_CTRL_7" tag = "65543" / >
<control - tag name = "TAB_CTRL_8" tag = "65544" / >
<control - tag name = "TAB_CTRL_9" tag = "65545" / >
<control - tag name = "TAB_CTRL_10" tag = "65546" / >
<control - tag name = "TAB_CTRL_11" tag = "65547" / >
<control - tag name = "TAB_CTRL_12" tag = "65548" / >
<control - tag name = "TAB_CTRL_13" tag = "65549" / >
<control - tag name = "TAB_CTRL_14" tag = "65550" / >
<control - tag name = "TAB_CTRL_15" tag = "65551" / >
<control - tag name = "TAB_CTRL_16" tag = "65552" / >
<control - tag name = "TAB_CTRL_17" tag = "65553" / >
<control - tag name = "TAB_CTRL_18" tag = "65554" / >
<control - tag name = "TAB_CTRL_19" tag = "65555" / >
<control - tag name = "TAB_CTRL_20" tag = "65556" / >
<control - tag name = "TAB_CTRL_21" tag = "65557" / >
<control - tag name = "TAB_CTRL_22" tag = "65558" / >
<control - tag name = "TAB_CTRL_23" tag = "65559" / >
<control - tag name = "TAB_CTRL_24" tag = "65560" / >
<control - tag name = "TAB_CTRL_25" tag = "65561" / >
<control - tag name = "TAB_CTRL_26" tag = "65562" / >
<control - tag name = "TAB_CTRL_27" tag = "65563" / >
<control - tag name = "TAB_CTRL_28" tag = "65564" / >
<control - tag name = "TAB_CTRL_29" tag = "65565" / >
<control - tag name = "TAB_CTRL_30" tag = "65566" / >
<control - tag name = "TAB_CTRL_31" tag = "65567" / >
< / control - tags>
<gradients>
<gradient name = "kLightGreyCColor">
<color - stop rgba = "#c8c8c8ff" start = "0" / >
<color - stop rgba = "#c8c8c8ff" start = "1" / >
< / gradient>
<gradient name = "kDarkGreyCColor">
<color - stop rgba = "#646464ff" start = "0" / >
<color - stop rgba = "#646464ff" start = "1" / >
< / gradient>
< / gradients>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 2" origin = "0, 0" size = "91 ,107" transparent = "true">
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "alphagreenableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "KRT" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "52" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "15 ,20" size = "57 ,49" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "KRT" custom - view - name = "UnitsEdit" default - value = "0" font = "~ NormalFontSmaller" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "5 ,75" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "UnitsLabel" default - value = "0.5" font = "~ NormalFont" font - antialias = "true" font - color = "~ GreenCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "-5 ,0" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "100 ,16" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "center" text - inset = "0, 0" title = "Reverb Gain" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 4" origin = "0, 0" size = "113 ,99" transparent = "true">
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "alphagreenableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "PD Atten" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "52" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "30 ,25" size = "55 ,47" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "PD Atten" custom - view - name = "UnitsEdit" default - value = "0" font = "~ NormalFontSmaller" font - antialias = "true" font - color = "~ BlackCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "20 ,75" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "UnitsLabel" default - value = "0.5" font = "~ NormalFont" font - antialias = "true" font - color = "~ MagentaCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "100 ,16" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "center" text - inset = "0, 0" title = "PreD Attenuation" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
< / template>
<template background - color = "~ GreyCColor" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" maxSize = "1024, 791" minSize = "1024, 791" mouse - enabled = "true" name = "Off Screen View 1" origin = "0, 0" size = "1024, 791" transparent = "false" / >
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx SliderGroup 1" origin = "0, 0" size = "158 ,297" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ YellowCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "35 ,15" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "83 ,35" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Reverb Time" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view background - offset = "0, 0" bitmap = "vsbackground" bitmap - offset = "0, 0" class = "CSlider" control - tag = "RT60" custom - view - name = "" default - value = "0" draw - back = "false" draw - back - color = "~ WhiteCColor" draw - frame = "false" draw - frame - color = "~ WhiteCColor" draw - value = "false" draw - value - color = "~ WhiteCColor" draw - value - from - center = "false" draw - value - inverted = "false" handle - bitmap = "vshandle" handle - offset = "-1, 0" max - value = "1" min - value = "0" mode = "free click" mouse - enabled = "true" orientation = "vertical" origin = "60 ,55" reverse - orientation = "false" size = "26 ,174" sub - controller = "" transparent = "false" transparent - handle = "true" wheel - inc - value = "0.1" zoom - factor = "10" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "RT60" custom - view - name = "" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "35 ,265" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx SliderGroup 0" origin = "0, 0" size = "109 ,95" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ NormalFont" font - antialias = "true" font - color = "~ BlueCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "5 ,0" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "93 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Reverb Pre-Delay" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view background - offset = "0, 0" bitmap = "vsbackshort56" bitmap - offset = "0, 0" class = "CSlider" control - tag = "Pre Delay Reverb" custom - view - name = "" default - value = "0" draw - back = "false" draw - back - color = "~ WhiteCColor" draw - frame = "false" draw - frame - color = "~ WhiteCColor" draw - value = "false" draw - value - color = "~ WhiteCColor" draw - value - from - center = "false" draw - value - inverted = "false" handle - bitmap = "vshandsmall56" handle - offset = "-1, 0" max - value = "1" min - value = "0" mode = "free click" mouse - enabled = "true" orientation = "vertical" origin = "40 ,19" reverse - orientation = "false" size = "21 ,56" sub - controller = "" transparent = "false" transparent - handle = "true" wheel - inc - value = "0.1" zoom - factor = "10" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Pre Delay Reverb" custom - view - name = "UnitsEdit" default - value = "0" font = "~ NormalFontSmaller" font - antialias = "true" font - color = "~ BlackCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,75" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,22" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 1" origin = "0, 0" size = "101 ,95" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "Orange" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0 ,5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "104 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Multi-Tap Delay" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "alphagreenableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "Multi-Tap Delay" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "52" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "25 ,25" size = "54 ,50" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "" custom - view - name = "UnitsEdit" default - value = "0" font = "~ NormalFontSmaller" font - antialias = "true" font - color = "~ BlackCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "-60 ,40" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Multi-Tap Delay" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "15, 80" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 0" origin = "0, 0" size = "75 ,85" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ BlueCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "-15 ,5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "100 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Rate" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "greyableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,25" size = "52 ,38" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Rate" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 67" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 3" origin = "0, 0" size = "78 ,85" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ MagentaCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0 ,0" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Depth" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "greyableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "Depth" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,20" size = "52 ,40" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Depth" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 67" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 5" origin = "0, 0" size = "81 ,85" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ GreenCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0 ,0" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Resonance" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "greyableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "Resonance" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,20" size = "52 ,40" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Resonance" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 67" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 6" origin = "0, 0" size = "85, 80" transparent = "true" sub - controller = "">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "Orange" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "-15 ,0" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "100 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Pre Delay" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "greyableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "Pre Delay" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,20" size = "52 ,40" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Pre Delay" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "-5 ,65" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 8" origin = "0, 0" size = "75 ,85" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ BlueCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0 ,0" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Pitch Emulator" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "blueableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "Pitch Emulator" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,20" size = "52 ,44" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Pitch Emulator" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 67" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 9" origin = "0, 0" size = "87 ,94" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ MagentaCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Dry/Wet" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "blueableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "Wet/Dry" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,20" size = "52 ,44" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Wet/Dry" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,70" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 10" origin = "0, 0" size = "91 ,101" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ GreenCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "BitRate" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "bitRate" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "blueableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "bitRate" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,30" size = "52 ,44" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "bitRate" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0 ,80" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 11" origin = "0, 0" size = "75 ,85" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "Orange" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "DownSampler" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "blueableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "Downsampling Ratio" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,20" size = "52 ,44" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Downsampling Ratio" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 67" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 12" origin = "0, 0" size = "75 ,85" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ YellowCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 5" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Bit Depth" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "Bit Depth" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "blueableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "Bit Depth" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "42" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "16, 22" size = "52 ,44" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Bit Depth" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 67" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
<template background - color = "~ GreyCColor" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" maxSize = "1024, 791" minSize = "1024, 791" mouse - enabled = "true" name = "Off Screen View 2" origin = "0, 0" size = "1024, 791" transparent = "false" / >
<template background - color = "~ GreyCColor" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" maxSize = "1024, 791" minSize = "1024, 791" mouse - enabled = "true" name = "Off Screen View 3" origin = "0, 0" size = "1024, 791" transparent = "false" / >
<template background - color = "~ GreyCColor" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" maxSize = "1024, 791" minSize = "1024, 791" mouse - enabled = "true" name = "Off Screen View 4" origin = "0, 0" size = "1024, 791" transparent = "false" / >
<template background - color = "" background - color - draw - style = "filled and stroked" bitmap = "" class = "CViewContainer" custom - view - name = "" mouse - enabled = "true" name = "Rafx KnobGroup 7" origin = "0, 0" size = "75 ,85" transparent = "true">
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextLabel" custom - view - name = "" default - value = "0.5" font = "~ SystemFont" font - antialias = "true" font - color = "~ YellowCColor" frame - color = "~ BlackCColor" frame - width = "1" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0 ,0" rafxlabel - type = "" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75 ,15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "false" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "Chorus Offset" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" control - tag = "" / >
<view angle - range = "270" angle - start = "135" background - offset = "0, 0" bitmap = "greyableton" circle - drawing = "false" class = "CAnimKnob" control - tag = "" corona - color = "~ WhiteCColor" corona - dash - dot = "false" corona - drawing = "false" corona - from - center = "false" corona - inset = "0" corona - inverted = "false" corona - outline = "false" custom - view - name = "" default - value = "0.5" handle - color = "~ WhiteCColor" handle - line - width = "1" handle - shadow - color = "~ GreyCColor" height - of - one - image = "52" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "10 ,20" size = "52 ,44" sub - controller = "" sub - pixmaps = "80" tooltip = "" transparent = "false" value - inset = "0" wheel - inc - value = "0.1" zoom - factor = "1.5" / >
<view back - color = "~ BlackCColor" background - offset = "0, 0" class = "CTextEdit" control - tag = "Chorus Offset" custom - view - name = "UnitsEdit" default - value = "0" font = "~ SystemFont" font - antialias = "true" font - color = "~ WhiteCColor" frame - color = "~ BlackCColor" frame - width = "1" immediate - text - change = "false" max - value = "1" min - value = "0" mouse - enabled = "true" origin = "0, 67" round - rect - radius = "6" shadow - color = "~ RedCColor" size = "75, 15" style - 3D - in = "false" style - 3D - out = "false" style - no - draw = "false" style - no - frame = "true" style - no - text = "false" style - round - rect = "false" style - shadow - text = "false" sub - controller = "" text - alignment = "" text - inset = "0, 0" title = "1234.56" tooltip = "" transparent = "true" value - precision = "2" wheel - inc - value = "0.1" / >
< / template>
< / vstgui - ui - description>