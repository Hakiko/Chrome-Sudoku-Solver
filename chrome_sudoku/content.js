/*function getBoard(cb) {
    var board_data = "";
    for (var i = 0; i < 9; ++i) {
        for (var j = 0; j < 9; ++j) {
            var name = "f" + String(j) + String(i);
            var field = document.getElementById(name);
            board_data += String(field.value || 0);
        }
    }
    var reply = {
        ok: true,
        board: board_data
    };
    console.log("sending reply", board_data);
    cb(reply);
}

function fillBoard(data, cb) {
    console.log("filling board");
    for (var i = 0; i < 9; ++i) {
        for (var j = 0; j < 9; ++j) {
            var name = "f" + String(j) + String(i);
            var field = document.getElementById(name);
            field.value = data[i * 9 + j];
        }
    }
    var reply = {
        ok: true
    };
    cb(reply);
}

window.onload = function() {
    if (document.getElementById("puzzle_grid") == null) {
        return;
    }
    chrome.runtime.onMessage.addListener(function(msg, sender, sendResponse) {
        console.log("got message:", msg);
        if (msg.action == "get_board") {
            getBoard(sendResponse);
        } else if (msg.action == "fill_board") {
            fillBoard(msg.board, sendResponse);
        } else {
            console.log("unknown action");
        }
    });
};*/

    if (document.getElementById("puzzle_grid") != null) {
        console.log("adding sudoku_click_port listener");
        chrome.runtime.onConnect.addListener(function(port_click){
            console.log("new connection");
            if (port_click.name == "sudoku_click_port") {
                console.log("new connection on sudoku_click_port");
                port_click.onMessage.addListener(function(message){
                    console.log("message from sudoku_click_port: ", message);
                });
                port = chrome.runtime.connect({
                    name: "sudoku_port"
                });
                port.onMessage.addListener(function(msg) {
                    if (msg.action == "get_board") {
                        console.log("get_board message received");
                        var board_data = "";
                        for (var i = 0; i < 9; ++i) {
                            for (var j = 0; j < 9; ++j) {
                                var name = "f" + String(j) + String(i);
                                var field = document.getElementById(name);
                                board_data += String(field.value || 0);
                            }
                        }
                        port.postMessage({
                            action: "get_board reply",
                            board: board_data
                        });
                    } else if (msg.action == "fill_board") {
                        console.log("fill_board message received");
                        data = msg.board;
                        for (var i = 0; i < 9; ++i) {
                            for (var j = 0; j < 9; ++j) {
                                var name = "f" + String(j) + String(i);
                                var field = document.getElementById(name);
                                field.value = data[i * 9 + j];
                            }
                        }
                        port.postMessage({
                            action: "fill_board reply"
                        });
                    }
                });
            }
        });
    }
