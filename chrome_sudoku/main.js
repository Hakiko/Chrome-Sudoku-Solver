function getFillBoardResponseCB() {
    return function(message) {
        console.log("fill board reponse:", message);
    }
}

function getNativeBoardResponseCB(tabId) {
    return function(response) {
        console.log("native response:", response);
        chrome.tabs.sendMessage(tabId, {action: "fill_board", board: response.board},
                getFillBoardResponseCB());
    }
}

function getBoardCB(tabId) {
    return function(message) {
        console.log("sending native message");
        chrome.runtime.sendNativeMessage("com.hayachi.native",
                {board: message.board}, getNativeBoardResponseCB(tabId));
    }
}

chrome.runtime.onConnect.addListener(function(port) {
    console.log("new connection");
    if (port.name == "sudoku_port") {
        console.log("new connection on sudoku_port");
        port.postMessage({
            action: "get_board"
        });
        port.onMessage.addListener(function(msg) {
            if (msg.action == "get_board reply") {
                console.log("got get_board reply");
                chrome.runtime.sendNativeMessage("com.hayachi.native",{
                    board: msg.board
                }, function(response) {
                    console.log("got native response: ", response);
                    port.postMessage({
                        action: "fill_board",
                        board: response.board
                    });
                });
            } else if (msg.action == "fill_board reply") {
                console.log("fill_board reply message:", msg);
            } else {
                console.log("unknown message");
            }
        });
    }
});

function clickCB() {
    chrome.tabs.query({active: true, currentWindow: true}, function(tabs) {
        var tab = tabs[0];
        //chrome.tabs.sendMessage(tab.id, {action: "get_board"}, getBoardCB(tab.id));
        console.log("connecting to sudoku_click_port");
        var port = chrome.tabs.connect(tab.id, {
            name: "sudoku_click_port"
        });
        port.onDisconnect.addListener(function(){
            console.log("pls no");
        });
        port.postMessage({

        });
    });

}

chrome.browserAction.onClicked.addListener(clickCB);
