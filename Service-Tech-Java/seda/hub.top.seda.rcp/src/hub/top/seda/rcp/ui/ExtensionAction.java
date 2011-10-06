/*****************************************************************************\
 * Copyright (c) 2009 Dirk Fahland. All rights reserved. EPL1.0/GPL3.0/LGPL3.0
 * 
 * ServiceTechnolog.org - RCP Application 
 * 
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License v1.0, which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is this file as it was released on April 06, 2009.
 * The Initial Developer of the Original Code are
 * 		Dirk Fahland
 * 
 * Portions created by the Initial Developer are Copyright (c) 2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the EPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the EPL, the GPL or the LGPL.
\*****************************************************************************/

package hub.top.seda.rcp.ui;

import java.net.MalformedURLException;
import java.net.URL;

import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.swt.custom.BusyIndicator;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.IWorkbenchWindowActionDelegate;
import org.eclipse.update.ui.UpdateJob;
import org.eclipse.update.ui.UpdateManagerUI;
import org.eclipse.update.search.BackLevelFilter;
import org.eclipse.update.search.EnvironmentFilter;
import org.eclipse.update.search.UpdateSearchRequest;
import org.eclipse.update.search.UpdateSearchScope;

/**
 * Find extensions for the current configuration
 * 
 * @author Dirk Fahland
 */
public class ExtensionAction implements IWorkbenchWindowActionDelegate {

	public void dispose() {
	}

	private IWorkbenchWindow window;
	
	public void init(IWorkbenchWindow window) {
		this.window = window;
	}

	public void run(IAction action) {
    BusyIndicator.showWhile(window.getShell().getDisplay(),
        new Runnable() {
          public void run() {
            UpdateJob job = new UpdateJob(
                "Search for new extensions", getSearchRequest());
            UpdateManagerUI.openInstaller(window.getShell(), job);
          }
        });
	}
	
  private UpdateSearchRequest getSearchRequest() {

    UpdateSearchScope scope = new UpdateSearchScope();
    try {
      String sedaHomeURL = System.getProperty("seda.updatesite", "http://download.gna.org/service-tech/seda/updatesite/");
      URL sedaUrl = new URL(sedaHomeURL);
      scope.addSearchSite("Seda Updates", sedaUrl, null);
      
      String emfHomeURL = System.getProperty("emf.updates", "http://download.eclipse.org/modeling/emf/updates/releases/");
      scope.addSearchSite("EMF Updates", new URL(emfHomeURL), null); 
      String gefHomeURL = System.getProperty("gef.updates", "http://download.eclipse.org/tools/gef/updates/releases/");
      scope.addSearchSite("GEF Updates", new URL(gefHomeURL), null);
      String gmfHomeURL = System.getProperty("gmf.updates", "http://download.eclipse.org/modeling/gmf/updates/releases/");
      scope.addSearchSite("GMF Updates", new URL(gmfHomeURL), null);
      
    } catch (MalformedURLException e) {
      // skip bad URLs
    }
    UpdateSearchRequest result = new UpdateSearchRequest(
        UpdateSearchRequest.createDefaultSiteSearchCategory(),
        scope);
    result.addFilter(new BackLevelFilter());
    result.addFilter(new EnvironmentFilter());
    return result;
  }


	public void selectionChanged(IAction action, ISelection selection) {
	}

}