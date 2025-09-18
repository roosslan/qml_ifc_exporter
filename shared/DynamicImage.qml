import QtQuick

Image
{
    source: "images/trash.png";
	//x: 765;
	width: 18;
	height: 18;
	MouseArea
	{
		anchors.fill: parent;
        onClicked: { parent.parent.destroy(); }
	}
}
