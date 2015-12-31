function main2() {
    var dialog = new NKWinTaskDialog();
    dialog.on("created", function (x) { alert("created"); alert.apply(this, arguments) });
    dialog.on("navigated", function (x) { alert("navigated"); alert.apply(this, arguments) });
    dialog.on("buttonClicked", function (x) { alert("buttonClicked"); alert.apply(this, arguments); /*dialog.preventClose();*/ });
    dialog.on("hyperlinkClicked", function (x) { alert("hyperlinkClicked"); alert.apply(this, arguments) });
    dialog.on("timer", function (x) { alert("timer"); alert.apply(this, arguments) });
    dialog.on("destroyed", function (x) { alert("destroyed"); alert.apply(this, arguments) });
    dialog.on("radioButtonClicked", function (x) { alert("radioButtonClicked"); alert.apply(this, arguments) });
    dialog.on("dialogConstructed", function (x) { alert("dialogConstructed"); alert.apply(this, arguments) });
    dialog.on("verificationClicked", function (x) { alert("verificationClicked"); alert.apply(this, arguments) });
    dialog.on("help", function (x) { alert("help"); alert.apply(this, arguments) });
    dialog.on("expandoButtonClicked", function (x) { alert("expandoButtonClicked"); alert.apply(this, arguments) });
    dialog.show({
        title: "囧好你叔",
        mainInstruction: "测试",
        icon: "warning",
        footer: "hahaha",
        footerIcon: "shield",
        content: "<a href=\"hello\">Lorem ipsum dolor sit amet</a>, consectetur adipiscing elit. Ut convallis porttitor pulvinar. Phasellus iaculis ullamcorper sem, nec tempus tortor mattis eu. Aliquam sodales, magna sit amet interdum facilisis, nibh purus lacinia dui, et interdum mi erat sed nisi. Quisque turpis tellus, facilisis vitae placerat quis, vehicula ac nibh. In finibus dui ac dictum congue. Nullam in porttitor libero. Vestibulum ac consectetur lacus. Aenean vehicula ut est nec dignissim. Aenean pharetra feugiat turpis a consectetur. Fusce consequat consectetur purus, quis tincidunt sapien auctor dapibus. Quisque ac magna sit amet sapien tincidunt imperdiet a eu est. Aenean at erat neque. Etiam id felis in leo elementum porttitor.",
        enableHyperlinks: true,
        showProgressBar: true,
        expandFooterArea: true,
        useCommandLinks: true,
        verificationText: "不要再显示这条三三",
        expandedInformation: "这里是三三 ><",
        expandedControlText: "三三快滚",
        collapsedControlText: "三三快来",
        buttons: [
            "prpr Misaka Mikoto\nNam in magna sapien. Vestibulum aliquet consectetur cursus. Ut libero metus, faucibus ac ex et, fermentum sodales tellus. Cras et pellentesque elit, mollis dapibus sem. Mauris egestas at ex sed aliquam. Mauris vulputate eu justo in pharetra. Donec at neque quis arcu commodo porta. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae;",
            "prpr Misaka 10952\nVivamus nunc velit, interdum in est at, condimentum ornare nunc. Mauris venenatis enim odio, a commodo nibh facilisis sit amet. Sed euismod mi est, quis tristique sem hendrerit in. Duis fermentum porta justo id scelerisque. Praesent ut diam est. Duis in blandit libero, non tristique erat. In hac habitasse platea dictumst.",
            "prpr Misaka 20001\nPraesent auctor aliquam sem. In feugiat egestas urna, quis imperdiet orci facilisis in. Donec vitae libero sit amet quam porttitor condimentum eu eu ante. Duis non lectus vel tellus aliquet porttitor ac at lacus. Vivamus fringilla enim vel erat blandit, id sodales ante venenatis. Duis non nulla bibendum libero bibendum vulputate eu at velit. Sed suscipit euismod leo. Morbi ultrices tempor porttitor.",
        ],
        radioButtons: [
            "prpr",
            "hshs",
            "????",
        ],
        commonButtons: ["retry", "close"]
    });
}

function msgBox(content) {
    var dialog = new NKWinTaskDialog();
    dialog.show({
        icon: "info",
        content: content,
        commonButtons: ["close"],
        allowDialogCancellation: true
    });
}
var msgbox = msgBox;

function main() {
    var func = Duktape.compile("3+3", "a.js", Duktape.compile.flagStrict);
    msgBox(func);
    msgBox(Duktape.enc('jc', Duktape.dumpBytecode(func)));
    msgBox(func());
    return;
    //msgBox(JSON.stringify(process, null, 4));

    var raw = fs.readFileSync("D:/NekoIM/更新履历.txt");
    var code, c;
    code = "extern const unsigned char _RMMVModEmbeddedResource[" + String(raw.length) + "] = {";
    for (var i = 0; i < raw.length; i++) {
        if (i % 16 == 0) code += "\n";
        c = "   " + String(raw[i])
        code += c.substring(c.length - 3) + ", "
    }
    code += "\n};"

    fs.writeFileSync("D:/NekoIM/更新履历.txt.233", code);
}

//new NKWinAPI("user32", "MessageBoxA")(0, "Hello from NKWinAPI.", "NKWinAPI", 16);

function main3() {
    var QueryPerformanceCounter = new NKWinAPI("kernel32", "QueryPerformanceCounter");

    function time() {
        var buf = new Buffer(8);
        QueryPerformanceCounter(buf);
        return buf;
    }

    var freq = (function() {
        var buf = new Buffer(8);
        new NKWinAPI("kernel32", "QueryPerformanceFrequency")(buf);
        return buf.readInt32LE(0);
    })();

    function timediff(a, b) {
        var diff = 0;
        var power = 1;
        for (var i = 0; i < 8; i++) {
            diff += (a[i] - b[i]) * power;
            power *= 256;
        }
        return diff / freq;
    }

    function benchmark() {
        var start = time();
        for (var i = 0; i < 1000000; i++) {
            1 + 1;
        }
        var end = time();
        return timediff(end, start);
    }

    var results = [benchmark(), benchmark(), benchmark(), benchmark(), benchmark(), benchmark(), benchmark(), benchmark(), benchmark(), benchmark()];
    msgbox(JSON.stringify(results, null, 4));
}

main3();