var path = {
    join: function () {
        return Array.prototype.slice.call(arguments).reduce(function (previousValue, currentValue) {
            if (!previousValue) return currentValue;

            if (previousValue[previousValue.length - 1] == path.sep)
                return previousValue + currentValue;
            else
                return previousValue + path.sep + currentValue;
        });
    },
    sep: '\\'
};

var hexPad = function (i) {
    return "0x" + ("0000000" + ((i | 0) + 4294967296).toString(16)).substr(-8);
};

var projectDir = process.argv[2];
var codeFile = path.join(projectDir, "Resource.generated.cpp");
var code = '';
var headerFile = path.join(projectDir, "Resource.generated.h");
var header = '';
var index = 0;
var offset = 0;
var blocks = [];
var blockTag = [];
var metas = '';

var addResource = function (name, content) {
    header += '#define NEKORES_' + name + '\t' + hexPad(index);
    metas += '    {' + hexPad(index) + ', ' + hexPad(offset) + ', ' + hexPad(content.length) + '}, //' + name + '\n';
    blocks.push(content);
    blockTag.push(name);
    offset += content.length + 1;
};

addResource("INIT_EVENT_EMITTER", fs.readFileSync(path.join(projectDir, "lib", "init_EventEmitter.js")));

code += 'const _NekoRes_Meta_t _NekoRes_Meta[] = {' + '\n';
code += metas;
code += '};' + '\n';

code += '\n';

var generateHexDump = function (data) {
    var result = [];
    for (var i = 0; i < data.length; i++) {
        if (i % 16 == 0) result.push("\n");
        var c = "   " + String(data[i]);
        result.push(c.substring(c.length - 3) + ", ");
    }
    return result.join("");
}

code += 'const char _NekoRes_Payload[] = {';
for (var i = 0; i < blocks.length; i++) {
    var block = blocks[i];
    code += '\n// ' + blockTag[i];
    code += generateHexDump(block);
    code += '\n  0,';
}
code += "\n};"

fs.writeFileSync(codeFile, code);
fs.writeFileSync(headerFile, header);
