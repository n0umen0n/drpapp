import { useState } from 'react';
import './App.css';
import { Button } from '@mui/joy';
import Sheet from '@mui/joy/Sheet';
import Modal from '@mui/joy/Modal';
import ModalClose from '@mui/joy/ModalClose';
import Typography from '@mui/joy/Typography';
import { DataContext } from './DataContext';
import { useContext } from 'react';

function Assistant() {
  const [open, setOpen] = useState(false);
  const { assistantState, page } = useContext(DataContext);
  const componentsMap = {
    CASE_VIEW: {
      title: <div>Case</div>, 
      content: <div>See details about the case. Depending on the state of the case and your relation to it, you can take different actions here.
        <ul>
          <li>If you are the respondent, you can acknowledge the case. If you've acknowledged it, you can counter the case or accept the accusations. If you accept the accusations, the case will be closed and the verdict is the initial claim against you.</li>
          <li>If you are the claimant you are able to view the progress of the case and submit additional evidence in this view.</li>
          <li>If you are an arbitrator for the case, then you have access to the arbitration view.</li>
        </ul>
      </div>
    },
    ALL_CASES: {
      title: <div>All cases</div>, 
      content: <div>Here you can see all cases in the selected community with their most basic information. You can open any case to take case specific actions.</div>
    },
    ARBITRATE: {
      title: <div>Arbitrate</div>, 
      content: <div>On this page, arbitrators can accept or reject the case, and the lead arbitrator can give a verdict.</div>
    },
    RESPOND_CASE: {
      title: <div>Respond case</div>, 
      content: <div>Here respond case</div>
    },
    ADD_COMMUNITY: {
      title: <div>Add community</div>, 
      content: <div>Here you can add a new community to the platform. Add the claim types that your community can use when creating or joining cases.</div>
    },
    BECOME_ARBITRATOR: {
      title: <div>Become arbitrator</div>, 
      content: <div>Currently there is no form for this. Just write us on telegram.</div>
    },
    GIVE_VERDICT: {
      title: <div>Give verdict</div>, 
      content: <div>As the lead arbitrator, you can now give a verdict. This verdict should be fully discussed and agreed upon with other arbitrators. After giving the verdict, other arbitrators shall sign it in order for the verdict to pass.</div>
    },
    ADD_EVIDENCE: {
      title: <div>Add evidence</div>, 
      content: <div>As long as the case is open, you can add or remove evidence and edit its description here.</div>
    },
    ACCEPT_CASE: {
      title: <div>Accept accusations</div>, 
      content: <div>If you accept the accusations, the case will be closed and you agree to the claims made against you.</div>
    },
    JOIN_CLAIM: {
      title: <div>Join claim</div>, 
      content: <div>Here you can join an existing case against someone. <br/> Provide:
      <ul>
        <li><b>A case description</b> - a full description of the case. Provide all the context, describe what happened.</li>
        <li><b>A list of your claims</b> - add a claim type, fine, relief, suspension. And then click add claim. Click on a claim to remove it. You can also request a ban, which is not specific to any claim.</li>
        <li><b>Contacts</b> - provide your telegram. If you have the respondents socials, then please add those.</li>
        <li><b>Proof</b> - you can add attachments to your case. This can be a screenshot, a video, a document, etc. The proof is uploaded to IPFS.</li>
        <li><b>Payment</b> - agree with the terms and conditions, and submit the case. The case will be submitted, along with your deposit payment.</li>
      </ul>
      </div>
    },
    ADD_CLAIM: {
      title: <div>Create a new case</div>, 
      content: <div>Here you can create a new case against someone. <br/> Provide:
      <ul>
        <li><b>A case description</b> - a full description of the case. Provide all the context, describe what happened.</li>
        <li><b>A list of your claims</b> - add a claim type, fine, relief, suspension. And then click add claim. Click on a claim to remove it. You can also request a ban, which is not specific to any claim.</li>
        <li><b>Number of arbitrators</b> - this is currently fixed.</li>
        <li><b>Contacts</b> - provide your telegram and the EOS account of the person you are creating a case against. If you have the respondents socials, then please add those.</li>
        <li><b>Proof</b> - you can add attachments to your case. This can be a screenshot, a video, a document, etc. The proof is uploaded to IPFS.</li>
        <li><b>Deposits</b> - to create a case, deposits must be paid. There is a minimum deposit. You can add a higher deposit. You also set the deposit for the repospondent that he has to pay after accepting the case. If you win the case you'll receive the deposit back. If respondent wins then he receives his deposit back. Loser pays his deposit to the arbitrators and the app. </li>
        <li><b>Payment</b> - agree with the terms and conditions, and submit the case. The case will be submitted, along with your deposit payment.</li>
      </ul>
      </div>
    }
  };
  return (
    <>
    <div className="assistantButton"><Button onClick={() => setOpen(true)}>Assistant</Button></div>
    <Modal
        aria-labelledby="modal-title"
        aria-describedby="modal-desc"
        open={open}
        onClose={() => setOpen(false)}
        sx={{ display: 'flex', justifyContent: 'center', alignItems: 'center', "margin":"5px"}}
      >
        <Sheet
          variant="outlined"
          sx={{
            maxWidth: 500,
            padding: "30px",
            paddingRight:"50px",
            borderRadius: 'md',
            boxShadow: 'lg',
          }}
        >
          <ModalClose variant="plain" sx={{ m: 1 }} />
          <Typography
            component="h2"
            id="modal-title"
            level="h4"
            textColor="inherit"
            fontWeight="lg"
            mb={1}
          >
            {componentsMap[page]?.title}
          </Typography>
          <Typography id="modal-desc" textColor="text.tertiary">
              {componentsMap[page]?.content}
          </Typography>
        </Sheet>
      </Modal>
      </>
  );
}
export default Assistant;