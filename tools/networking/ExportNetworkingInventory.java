// Read-only function metadata census; no decompilation, analysis, or program edits.
// @category HD2

import com.google.gson.GsonBuilder;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import ghidra.app.script.GhidraScript;
import ghidra.framework.Application;
import ghidra.program.model.address.AddressRange;
import ghidra.program.model.address.AddressRangeIterator;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;
import ghidra.program.model.listing.FunctionTag;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.TreeMap;
import java.util.TreeSet;

public class ExportNetworkingInventory extends GhidraScript {
    private final TreeMap<String, Function> functions = new TreeMap<>();

    private void collect(FunctionIterator iterator) throws Exception {
        while (iterator.hasNext()) {
            monitor.checkCancelled();
            Function function = iterator.next();
            functions.put(function.getEntryPoint().toString(), function);
        }
    }

    @Override
    public void run() throws Exception {
        String hash = currentProgram.getExecutableSHA256();
        String program;
        if ("48ba25814ab429fed4418d8610b2eca5aaeb594d83a29a0ca6b99fa152f1c152".equals(hash)) {
            program = "server";
        } else if ("6ed896ce1b468b04ce3d0136f9d4b51340c648838bd9a2eb34761cc2a1c96fbb".equals(hash)) {
            program = "launcher";
        } else {
            throw new IllegalArgumentException("Only fingerprinted HD2DS and ServerLauncher inputs allowed");
        }
        String[] args = getScriptArgs();
        if (args.length != 1) {
            throw new IllegalArgumentException("Supply output JSON beneath this workspace's analysis/work");
        }
        Path work = Path.of("analysis/work").toRealPath();
        Path output = Path.of(args[0]).toAbsolutePath().normalize();
        if (!output.startsWith(work) || !output.getParent().toRealPath().startsWith(work)
                || Files.isSymbolicLink(output)) {
            throw new IllegalArgumentException("Output must be a local non-symlink file beneath analysis/work");
        }
        collect(currentProgram.getFunctionManager().getFunctions(true));
        collect(currentProgram.getFunctionManager().getExternalFunctions());
        JsonObject inventory = new JsonObject();
        inventory.addProperty("schema_version", 1);
        inventory.addProperty("program", program);
        inventory.addProperty("program_name", currentProgram.getName());
        inventory.addProperty("sha256", hash);
        inventory.addProperty("ghidra_version", Application.getApplicationVersion());
        inventory.addProperty("image_base", currentProgram.getImageBase().toString());
        JsonArray records = new JsonArray();
        for (Function function : functions.values()) {
            monitor.checkCancelled();
            JsonObject record = new JsonObject();
            record.addProperty("entry", function.getEntryPoint().toString());
            record.addProperty("name", function.getName());
            record.addProperty("namespace", function.getParentNamespace().getName(true));
            record.addProperty("name_source", function.getSymbol().getSource().toString());
            record.addProperty("external", function.isExternal());
            record.addProperty("thunk", function.isThunk());
            record.addProperty("body_bytes", function.getBody().getNumAddresses());
            record.addProperty("classification", function.isExternal() ? "external"
                : function.isThunk() ? "internal_thunk" : "unclassified");
            JsonArray tags = new JsonArray();
            TreeSet<String> sortedTags = new TreeSet<>();
            for (FunctionTag tag : function.getTags()) {
                sortedTags.add(tag.getName());
            }
            for (String tag : sortedTags) {
                tags.add(tag);
            }
            record.add("tags", tags);
            JsonArray ranges = new JsonArray();
            if (!function.isExternal()) {
                AddressRangeIterator iterator = function.getBody().getAddressRanges(true);
                while (iterator.hasNext()) {
                    AddressRange range = iterator.next();
                    JsonArray pair = new JsonArray();
                    pair.add(range.getMinAddress().toString());
                    pair.add(range.getMaxAddress().toString());
                    ranges.add(pair);
                }
            }
            record.add("body_ranges", ranges);
            if (function.isThunk()) {
                Function target = function.getThunkedFunction(true);
                if (target != null) {
                    record.addProperty("thunk_target", target.getEntryPoint().toString());
                }
            }
            records.add(record);
        }
        inventory.add("functions", records);
        Files.writeString(output, new GsonBuilder().setPrettyPrinting().create().toJson(inventory) + "\n",
            StandardCharsets.UTF_8);
        println("INVENTORY " + program + " " + records.size() + " identified entries -> " + output);
    }
}
