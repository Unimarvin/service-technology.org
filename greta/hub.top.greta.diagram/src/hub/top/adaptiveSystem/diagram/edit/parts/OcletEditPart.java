package hub.top.adaptiveSystem.diagram.edit.parts;

import hub.top.adaptiveSystem.AdaptiveSystemPackage;
import hub.top.adaptiveSystem.Oclet;

import org.eclipse.draw2d.ColorConstants;
import org.eclipse.draw2d.GridData;
import org.eclipse.draw2d.GridLayout;
import org.eclipse.draw2d.IFigure;
import org.eclipse.draw2d.LineBorder;
import org.eclipse.draw2d.RectangleFigure;
import org.eclipse.draw2d.StackLayout;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.ecore.impl.EAttributeImpl;
import org.eclipse.gef.EditPart;
import org.eclipse.gef.EditPolicy;
import org.eclipse.gef.Request;
import org.eclipse.gef.commands.Command;
import org.eclipse.gef.editpolicies.LayoutEditPolicy;
import org.eclipse.gef.editpolicies.NonResizableEditPolicy;
import org.eclipse.gef.requests.CreateRequest;
import org.eclipse.gmf.runtime.diagram.ui.editparts.IGraphicalEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editparts.ShapeNodeEditPart;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.CreationEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.DragDropEditPolicy;
import org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles;
import org.eclipse.gmf.runtime.draw2d.ui.figures.ConstrainedToolbarLayout;
import org.eclipse.gmf.runtime.draw2d.ui.figures.WrappingLabel;
import org.eclipse.gmf.runtime.gef.ui.figures.DefaultSizeNodeFigure;
import org.eclipse.gmf.runtime.gef.ui.figures.NodeFigure;
import org.eclipse.gmf.runtime.notation.Node;
import org.eclipse.gmf.runtime.notation.View;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.Display;

/**
 * @generated
 */
public class OcletEditPart extends ShapeNodeEditPart {

	/**
	 * @generated
	 */
	public static final int VISUAL_ID = 1002;

	/**
	 * @generated
	 */
	protected IFigure contentPane;

	/**
	 * @generated
	 */
	protected IFigure primaryShape;

	/**
	 * @generated
	 */
	public OcletEditPart(View view) {
		super(view);
	}

	/**
	 * @generated
	 */
	protected void createDefaultEditPolicies() {
		installEditPolicy(EditPolicyRoles.CREATION_ROLE,
				new CreationEditPolicy());
		super.createDefaultEditPolicies();
		installEditPolicy(
				EditPolicyRoles.SEMANTIC_ROLE,
				new hub.top.adaptiveSystem.diagram.edit.policies.OcletItemSemanticEditPolicy());
		installEditPolicy(EditPolicyRoles.DRAG_DROP_ROLE,
				new DragDropEditPolicy());
		installEditPolicy(
				EditPolicyRoles.CANONICAL_ROLE,
				new hub.top.adaptiveSystem.diagram.edit.policies.OcletCanonicalEditPolicy());
		installEditPolicy(EditPolicy.LAYOUT_ROLE, createLayoutEditPolicy());
		// XXX need an SCR to runtime to have another abstract superclass that would let children add reasonable editpolicies
		// removeEditPolicy(org.eclipse.gmf.runtime.diagram.ui.editpolicies.EditPolicyRoles.CONNECTION_HANDLES_ROLE);
	}

	/**
	 * @generated
	 */
	protected LayoutEditPolicy createLayoutEditPolicy() {
		LayoutEditPolicy lep = new LayoutEditPolicy() {

			protected EditPolicy createChildEditPolicy(EditPart child) {
				EditPolicy result = child
						.getEditPolicy(EditPolicy.PRIMARY_DRAG_ROLE);
				if (result == null) {
					result = new NonResizableEditPolicy();
				}
				return result;
			}

			protected Command getMoveChildrenCommand(Request request) {
				return null;
			}

			protected Command getCreateCommand(CreateRequest request) {
				return null;
			}
		};
		return lep;
	}

	/**
	 * @generated
	 */
	protected IFigure createNodeShape() {
		OcletDescriptor figure = new OcletDescriptor();
		return primaryShape = figure;
	}

	/**
	 * @generated
	 */
	public OcletDescriptor getPrimaryShape() {
		return (OcletDescriptor) primaryShape;
	}

	/**
	 * @generated
	 */
	protected boolean addFixedChild(EditPart childEditPart) {
		if (childEditPart instanceof hub.top.adaptiveSystem.diagram.edit.parts.OcletNameEditPart) {
			((hub.top.adaptiveSystem.diagram.edit.parts.OcletNameEditPart) childEditPart)
					.setLabel(getPrimaryShape().getFigureOcletName());
			return true;
		}
		if (childEditPart instanceof hub.top.adaptiveSystem.diagram.edit.parts.OcletQuantorOrientationEditPart) {
			((hub.top.adaptiveSystem.diagram.edit.parts.OcletQuantorOrientationEditPart) childEditPart)
					.setLabel(getPrimaryShape().getFigureOcletQuantor());
			return true;
		}
		return false;
	}

	/**
	 * @generated
	 */
	protected boolean removeFixedChild(EditPart childEditPart) {

		return false;
	}

	/**
	 * @generated
	 */
	protected void addChildVisual(EditPart childEditPart, int index) {
		if (addFixedChild(childEditPart)) {
			return;
		}
		super.addChildVisual(childEditPart, -1);
	}

	/**
	 * @generated
	 */
	protected void removeChildVisual(EditPart childEditPart) {
		if (removeFixedChild(childEditPart)) {
			return;
		}
		super.removeChildVisual(childEditPart);
	}

	/**
	 * @generated
	 */
	protected IFigure getContentPaneFor(IGraphicalEditPart editPart) {

		return super.getContentPaneFor(editPart);
	}

	/**
	 * @generated
	 */
	protected NodeFigure createNodePlate() {
		DefaultSizeNodeFigure result = new DefaultSizeNodeFigure(getMapMode()
				.DPtoLP(200), getMapMode().DPtoLP(200));
		return result;
	}

	/**
	 * Creates figure for this edit part.
	 * 
	 * Body of this method does not depend on settings in generation model
	 * so you may safely remove <i>generated</i> tag and modify it.
	 * 
	 * @generated
	 */
	protected NodeFigure createNodeFigure() {
		NodeFigure figure = createNodePlate();
		figure.setLayoutManager(new StackLayout());
		IFigure shape = createNodeShape();
		figure.add(shape);
		contentPane = setupContentPane(shape);
		return figure;
	}

	/**
	 * Default implementation treats passed figure as content pane.
	 * Respects layout one may have set for generated figure.
	 * @param nodeShape instance of generated figure class
	 * @generated NOT
	 */
	protected IFigure setupContentPane(IFigure nodeShape) {
		if (nodeShape.getLayoutManager() == null) {
			ConstrainedToolbarLayout layout = new ConstrainedToolbarLayout();
			layout.setSpacing(getMapMode().DPtoLP(0));
			nodeShape.setLayoutManager(layout);
		}
		return nodeShape; // use nodeShape itself as contentPane
	}

	/**
	 * @generated
	 */
	public IFigure getContentPane() {
		if (contentPane != null) {
			return contentPane;
		}
		return super.getContentPane();
	}

	/**
	 * @generated
	 */
	public EditPart getPrimaryChildEditPart() {
		return getChildBySemanticHint(hub.top.adaptiveSystem.diagram.part.AdaptiveSystemVisualIDRegistry
				.getType(hub.top.adaptiveSystem.diagram.edit.parts.OcletNameEditPart.VISUAL_ID));
	}

	/**
	 * @author Manja Wolf
	 */
	@Override
	protected void handleNotificationEvent(Notification notification) {
		Object feature = notification.getFeature();
		if (notification.getNotifier() instanceof Oclet) {

			//if the attribute wellformed changed, then recolor oclet
			if (feature instanceof EAttributeImpl) {
				EAttributeImpl attribute = (EAttributeImpl) feature;
				if (attribute.getName().equals(
						AdaptiveSystemPackage.eINSTANCE.getOclet_WellFormed()
								.getName())) {
					this.getPrimaryShape().updateFace();
				}
			}
		}
		super.handleNotificationEvent(notification);
	}

	/**
	 * @generated
	 */
	public class OcletDescriptor extends RectangleFigure {

		/**
		 * @generated
		 */
		private WrappingLabel fFigureOcletName;
		/**
		 * @generated
		 */
		private WrappingLabel fFigureOcletQuantor;
		/**
		 * @generated
		 */
		private RectangleFigure fFigureOcletBody;
		/**
		 * @generated
		 */
		private WrappingLabel fFigureOcletOrientation;

		/**
		 * @generated NOT
		 */
		public OcletDescriptor() {
			this.setFill(false);
			this.setPreferredSize(new Dimension(getMapMode().DPtoLP(200),
					getMapMode().DPtoLP(200)));
			this.setMaximumSize(new Dimension(getMapMode().DPtoLP(1000),
					getMapMode().DPtoLP(1000)));
			this.setMinimumSize(new Dimension(getMapMode().DPtoLP(200),
					getMapMode().DPtoLP(100)));
			updateFace();
			createContents();
		}

		/**
		 * @generated NOT
		 */
		private void createContents() {

			RectangleFigure labelContainer0 = new RectangleFigure();
			labelContainer0.setFill(false);
			labelContainer0.setOutline(false);
			labelContainer0.setForegroundColor(ColorConstants.orange);
			labelContainer0.setPreferredSize(new Dimension(getMapMode().DPtoLP(
					200), getMapMode().DPtoLP(20)));
			labelContainer0.setMaximumSize(new Dimension(getMapMode().DPtoLP(
					1000), getMapMode().DPtoLP(20)));
			labelContainer0.setMinimumSize(new Dimension(getMapMode().DPtoLP(
					200), getMapMode().DPtoLP(20)));

			this.add(labelContainer0);

			GridLayout layoutLabelContainer0 = new GridLayout();
			layoutLabelContainer0.numColumns = 3;
			layoutLabelContainer0.makeColumnsEqualWidth = false;
			//START: Manja Wolf
			layoutLabelContainer0.marginHeight = 2;
			layoutLabelContainer0.marginWidth = 0;
			layoutLabelContainer0.verticalSpacing = 0;
			//END: Manja Wolf
			labelContainer0.setLayoutManager(layoutLabelContainer0);

			fFigureOcletName = new WrappingLabel();
			fFigureOcletName.setText("");

			fFigureOcletName.setFont(FFIGUREOCLETNAME_FONT);

			GridData constraintFFigureOcletName = new GridData();
			constraintFFigureOcletName.verticalAlignment = SWT.CENTER;
			constraintFFigureOcletName.horizontalAlignment = GridData.BEGINNING;
			constraintFFigureOcletName.horizontalIndent = 10;
			constraintFFigureOcletName.horizontalSpan = 1;
			constraintFFigureOcletName.verticalSpan = 1;
			constraintFFigureOcletName.grabExcessHorizontalSpace = false;
			constraintFFigureOcletName.grabExcessVerticalSpace = false;
			labelContainer0.add(fFigureOcletName, constraintFFigureOcletName);

			fFigureOcletQuantor = new WrappingLabel();
			fFigureOcletQuantor.setText("");

			GridData constraintFFigureOcletQuantor = new GridData();
			constraintFFigureOcletQuantor.verticalAlignment = SWT.CENTER;
			constraintFFigureOcletQuantor.horizontalAlignment = GridData.BEGINNING;
			constraintFFigureOcletQuantor.horizontalIndent = 0;
			constraintFFigureOcletQuantor.horizontalSpan = 1;
			constraintFFigureOcletQuantor.verticalSpan = 1;
			constraintFFigureOcletQuantor.grabExcessHorizontalSpace = false;
			constraintFFigureOcletQuantor.grabExcessVerticalSpace = false;
			labelContainer0.add(fFigureOcletQuantor,
					constraintFFigureOcletQuantor);

			fFigureOcletOrientation = new WrappingLabel();
			fFigureOcletOrientation.setText("");

			GridData constraintFFigureOcletOrientation = new GridData();
			constraintFFigureOcletOrientation.verticalAlignment = SWT.CENTER;
			constraintFFigureOcletOrientation.horizontalAlignment = GridData.BEGINNING;
			constraintFFigureOcletOrientation.horizontalIndent = 0;
			constraintFFigureOcletOrientation.horizontalSpan = 1;
			constraintFFigureOcletOrientation.verticalSpan = 1;
			constraintFFigureOcletOrientation.grabExcessHorizontalSpace = false;
			constraintFFigureOcletOrientation.grabExcessVerticalSpace = false;
			labelContainer0.add(fFigureOcletOrientation,
					constraintFFigureOcletOrientation);

			fFigureOcletBody = new RectangleFigure();
			fFigureOcletBody.setFill(false);
			fFigureOcletBody.setPreferredSize(new Dimension(getMapMode()
					.DPtoLP(200), getMapMode().DPtoLP(1)));
			fFigureOcletBody.setMaximumSize(new Dimension(getMapMode().DPtoLP(
					1000), getMapMode().DPtoLP(1)));

			this.add(fFigureOcletBody);

		}

		/**
		 * @generated
		 */
		private boolean myUseLocalCoordinates = false;

		/**
		 * @generated
		 */
		protected boolean useLocalCoordinates() {
			return myUseLocalCoordinates;
		}

		/**
		 * @generated
		 */
		protected void setUseLocalCoordinates(boolean useLocalCoordinates) {
			myUseLocalCoordinates = useLocalCoordinates;
		}

		/**
		 * @generated
		 */
		public WrappingLabel getFigureOcletName() {
			return fFigureOcletName;
		}

		/**
		 * @generated
		 */
		public WrappingLabel getFigureOcletQuantor() {
			return fFigureOcletQuantor;
		}

		/**
		 * @generated
		 */
		public RectangleFigure getFigureOcletBody() {
			return fFigureOcletBody;
		}

		/**
		 * @generated
		 */
		public WrappingLabel getFigureOcletOrientation() {
			return fFigureOcletOrientation;
		}

		/**
		 * @author Manja Wolf
		 */
		public void updateFace() {
			Oclet oclet = (Oclet) ((Node) OcletEditPart.this.getModel())
					.getElement();

			if (oclet.isWellFormed()) {
				this.setBorder(new LineBorder(new Color(null, 30, 204, 30),
						getMapMode().DPtoLP(2)));
			} else {
				this.setBorder(null);
			}

		}
	}

	/**
	 * @generated
	 */
	static final Font FFIGUREOCLETNAME_FONT = new Font(Display.getCurrent(),
			Display.getDefault().getSystemFont().getFontData()[0].getName(), 9,
			SWT.BOLD);

}
