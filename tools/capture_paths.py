"""Keep disposable renderer frames in build; promote only reviewed docs assets."""
import argparse
from pathlib import Path
import shutil


def caption_font():
    """Use Pillow's classic bitmap caption font across supported versions.

    Pillow 9.3 returned this font from load_default(); Pillow 12.3 returns a
    FreeType font there and exposes the original as load_default_imagefont().
    Captions are outside the production framebuffer, whose gint atlas stays
    unchanged. Both installed implementations render identical bitmap pixels.
    """
    from PIL import ImageFont
    bitmap_loader = getattr(ImageFont, "load_default_imagefont", None)
    if bitmap_loader is not None:
        return bitmap_loader()
    return ImageFont.load_default()


class CapturePaths:
    """Shared destinations, without changing each suite's rendering scenarios."""

    def __init__(self, script, suite):
        parser = argparse.ArgumentParser(description=__doc__)
        parser.add_argument(
            "--update-docs", action="store_true",
            help="copy the retained gallery images to their canonical source paths")
        self.options = parser.parse_args()
        self.root = Path(script).resolve().parents[1]
        self.suite = suite
        self.output = self.root / "build" / "captures" / suite
        self.temporary_root = self.root / "build" / "tmp" / "captures"
        self.output.mkdir(parents=True, exist_ok=True)
        self.temporary_root.mkdir(parents=True, exist_ok=True)
        self.app = self.root / "build" / "host" / "host_app"
        self.menu_test = self.root / "build" / "host" / "test_menu"

    def finish(self):
        """Explicitly promote a small, fixed gallery rather than every PNG."""
        if not self.options.update_docs:
            print(f"Review images: {self.output.relative_to(self.root)}; "
                  "use --update-docs to refresh the retained gallery.")
            return
        current = "docs/captures/"
        retained = {
            "ui": {"host-overview.png": current + "host-overview.png",
                   "workflow-overview.png": current + "workflow-overview.png"},
            "power": {"power-overview.png": current + "power-overview.png"},
            "visibility": {"visibility-overview.png": current + "visibility-overview.png"},
            # Archived sheets are frozen milestone evidence. Their current
            # scenarios still render to build, but never overwrite the record.
            "audit": {},
            "overlays": {},
            "consistency": {},
            "interaction": {},
            "tiles": {"tiles-overview.png": current + "tiles-overview.png",
                      "tiles-main-first.png": current + "tiles-main-first.png",
                      "tiles-subtype-first.png": current + "tiles-subtype-first.png"},
            "phase": {name + ".png": current + name + ".png" for name in
                      ("phase-system-input", "phase-field", "phase-equilibrium")},
            "events": {name + ".png": current + name + ".png" for name in
                       ("event-settings", "solver-diagnostics")},
            "readme": {name + ".png": current + name + ".png" for name in
                       ("equation-entry", "initial-conditions", "solver-parameters",
                        "graph-solution", "graph-slope-field", "graph-trace",
                        "graph-gsolve", "table-view", "diffeq-icon")},
        }[self.suite]
        if self.suite == "tiles":
            for name in ("first", "second", "higher", "system", "separable",
                         "linear", "bernoulli", "others"):
                retained["menu/" + name + ".png"] = "assets/menu/" + name + ".png"
        for source, target in retained.items():
            destination = self.root / target
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(self.output / source, destination)
        print(f"Updated {len(retained)} retained gallery images.")
