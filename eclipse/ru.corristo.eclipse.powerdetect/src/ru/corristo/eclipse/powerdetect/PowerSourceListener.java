package ru.corristo.eclipse.powerdetect;

import org.eclipse.core.resources.IWorkspace;
import org.eclipse.core.resources.IWorkspaceDescription;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.swt.widgets.Display;

public class PowerSourceListener {
	/**
	 * Should be called from native module when power source actually changed
	 * 
	 * @param isBattery
	 *            is we are on battery power
	 */
	public static void powerSourceChanged(final boolean isBattery) {
		System.out.println("Power source changed to "
				+ (isBattery ? "battery" : "AC"));

		try {

			Display.getDefault().asyncExec(new Runnable() {

				@Override
				public void run() {
					try {
						IWorkspace workspace = ResourcesPlugin.getWorkspace();
						IWorkspaceDescription description = workspace
								.getDescription();
						description.setAutoBuilding(!isBattery);
						workspace.setDescription(description);
					} catch (CoreException e) {
						e.printStackTrace();
					}
				}
			});

		} catch (Throwable e) {
			e.printStackTrace();
		}
	}
}
