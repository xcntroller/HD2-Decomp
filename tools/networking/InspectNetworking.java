// Read-only local evidence: defined strings/imports and their containing callers.
// @category HD2
import com.google.gson.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.nio.file.*;

public class InspectNetworking extends GhidraScript {
    private JsonArray references(ghidra.program.model.address.Address address) {
        JsonArray out = new JsonArray();
        for (Reference ref : getReferencesTo(address)) {
            JsonObject row = new JsonObject();
            row.addProperty("from", ref.getFromAddress().toString());
            Function function = getFunctionContaining(ref.getFromAddress());
            if (function != null) row.addProperty("function", function.getEntryPoint().toString());
            out.add(row);
        }
        return out;
    }
    public void run() throws Exception {
        JsonObject out = new JsonObject();
        out.addProperty("sha256", currentProgram.getExecutableSHA256());
        JsonArray strings = new JsonArray(), imports = new JsonArray();
        DataIterator data = currentProgram.getListing().getDefinedData(true);
        while (data.hasNext()) {
            Data item = data.next();
            if (!item.hasStringValue()) continue;
            JsonObject row = new JsonObject();
            row.addProperty("address", item.getAddress().toString());
            row.addProperty("value", item.getValue().toString());
            row.add("references", references(item.getAddress()));
            strings.add(row);
        }
        SymbolIterator symbols = currentProgram.getSymbolTable().getAllSymbols(true);
        while (symbols.hasNext()) {
            Symbol symbol = symbols.next();
            if (!symbol.getName().startsWith("PTR_") && !symbol.isExternal()) continue;
            JsonObject row = new JsonObject();
            row.addProperty("name", symbol.getName());
            row.addProperty("address", symbol.getAddress().toString());
            row.add("references", references(symbol.getAddress()));
            imports.add(row);
        }
        out.add("strings", strings); out.add("imports", imports);
        Files.writeString(Path.of(getScriptArgs()[0]), new GsonBuilder().setPrettyPrinting().create().toJson(out),
            StandardOpenOption.CREATE_NEW);
    }
}
