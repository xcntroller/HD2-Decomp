// Find defined resource strings and their saved cross-references, read-only.
// @category HD2

import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Data;
import ghidra.program.model.listing.DataIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.regex.Pattern;

public class FindResourceReferences extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length != 2) {
            throw new IllegalArgumentException("Supply a new output filename and string regex");
        }
        Pattern pattern = Pattern.compile(args[1], Pattern.CASE_INSENSITIVE);
        JsonArray matches = new JsonArray();
        DataIterator data = currentProgram.getListing().getDefinedData(true);
        while (data.hasNext()) {
            monitor.checkCancelled();
            Data item = data.next();
            if (!item.hasStringValue()) {
                continue;
            }
            String value = item.getValue().toString();
            if (!pattern.matcher(value).find()) {
                continue;
            }
            JsonObject match = new JsonObject();
            match.addProperty("address", item.getAddress().toString());
            match.addProperty("string", value);
            JsonArray references = new JsonArray();
            ReferenceIterator refs =
                currentProgram.getReferenceManager().getReferencesTo(item.getAddress());
            while (refs.hasNext()) {
                Reference reference = refs.next();
                JsonObject ref = new JsonObject();
                ref.addProperty("from", reference.getFromAddress().toString());
                ref.addProperty("type", reference.getReferenceType().toString());
                Function function = getFunctionContaining(reference.getFromAddress());
                if (function != null) {
                    ref.addProperty("function", function.getEntryPoint().toString());
                    ref.addProperty("name", function.getName());
                }
                references.add(ref);
            }
            match.add("references", references);
            matches.add(match);
        }
        Files.writeString(Path.of(args[0]),
            new GsonBuilder().setPrettyPrinting().create().toJson(matches),
            StandardCharsets.UTF_8, StandardOpenOption.CREATE_NEW);
        println("Exported " + matches.size() + " matching resource strings");
    }
}
